#include "CHGWC.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>

#include "Packets.h"
#include "Log.h"
#include "detours.h"

//CRASH 004063A9
#pragma comment(lib, "detours.lib")

HANDLE CreateS4Client(std::string strPath, std::string strCommandLine)
{
	STARTUPINFOA startupInfo = {0};
	PROCESS_INFORMATION processInfo = {0};

	startupInfo.cb = sizeof(STARTUPINFOA);

	if(!CreateProcessA(strPath.c_str(), const_cast<LPSTR>(strCommandLine.c_str()), nullptr, nullptr, FALSE, CREATE_DEFAULT_ERROR_MODE, nullptr, nullptr, &startupInfo, &processInfo))
		return INVALID_HANDLE_VALUE;

	return processInfo.hProcess;
}

CHGWC *CHGWC::m_pSingleton = nullptr;

CHGWC* CHGWC::Singleton()
{
	if(!m_pSingleton)
		m_pSingleton = new CHGWC();

	return m_pSingleton;
}

DWORD WINAPI HandlePacketExGate(LPVOID lpParam)
{
	((CHGWC*)lpParam)->HandlePacketEx();

	return TRUE;
}

CHGWC::CHGWC()
{
	WSADATA wsaData;

	if(WSAStartup(MAKEWORD(2, 2) ,&wsaData))
		throw std::runtime_error("WSAStartup failed");

	m_bIgnorePackets = FALSE;
	m_uiSendCounter = 1;
	m_uiCrcSize = 0;
}

CHGWC::~CHGWC()
{
	closesocket(m_sClientSocket);
	WSACleanup();
}

void CHGWC::Init(std::string strIP, unsigned int ushPort)
{
	SOCKADDR_IN sAddrServer = {0};
	BYTE pbShortKey[] =
	{
		0x00, 0x01, 0xEE, 0x03,
		0xBB, 0x05, 0x10, 0x11,
		0x15, 0x2A, 0x99, 0xAA,
		0x04, 0xCC, 0x02, 0xFF
	};

	//Create socket
	m_sClientSocket = socket(AF_INET, SOCK_STREAM, 0);

	if(m_sClientSocket == SOCKET_ERROR)
	{
		throw std::exception("socket() failed!");
		return;
	}

	//Connect to server
	sAddrServer.sin_family = AF_INET;
	sAddrServer.sin_port = htons(ushPort);
	sAddrServer.sin_addr.s_addr = inet_addr(strIP.c_str());

	if(connect(m_sClientSocket, (SOCKADDR*)&sAddrServer, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
		throw std::exception("Failed to connect");
		return;
	}

	std::cout << "Connected to HGWC Server : " << strIP.c_str() << " : " << ushPort << "." << std::endl;

	//Prepare Encryption
	SEEDMakeCryptKey(pbShortKey, m_pbLongKey);

	PrepareConnection();

}

void CHGWC::PrepareConnection()
{
	////Create main handle thread
	//CreateThread(NULL, NULL, HandlePacketExGate, (void*)this, 0, 0);

	//send C_HAND_SHAKE
	SendEncryptedPacket(pbHandShakePacket, 96);
	std::cout << "C_HAND_SHAKE sent." << std::endl;


	HandlePacketEx();

}

void CHGWC::HandlePacketEx()
{
	char szRecvBuffer[512] = {0};
	DWORD dwRecvdBytes = 0;
	std::vector<BYTE> vRecvVec;
	std::vector<u_short> vRecvLen;

	while(TRUE)
	{
		dwRecvdBytes = recv(m_sClientSocket, szRecvBuffer, 512, 0);

		if(dwRecvdBytes == -1 || dwRecvdBytes == 0)
		{
			std::cout << "Server disconnected." << std::endl;
			std::cin.get();
		}
		else
		{
			DecryptPacket((PBYTE)szRecvBuffer, dwRecvdBytes);

			if(dwRecvdBytes >= 12)
			{
				for(int i = 0; i < dwRecvdBytes; ++i)
				{
					if(*(BYTE*)(szRecvBuffer + i) == 0x6D && *(BYTE*)(szRecvBuffer + i + 1) == 0xA5 && *(BYTE*)(szRecvBuffer + i + 2) == 0x03)
					{
						EncryptPacket((PBYTE)(szRecvBuffer + i), dwRecvdBytes - i);
						HandlePacket((PBYTE)(szRecvBuffer + i), dwRecvdBytes - i);
					}
				}
			}
		}
	}

}

void CHGWC::HandlePacket(PBYTE pbData, unsigned short ushLen)
{
	std::vector<BYTE> vSendVector;
	BYTE pbShortKey[16] = {0};
	char szBuffer[10] = {0};
	std::ostringstream ostrStream;
	int iColLen = 0;

	DecryptPacket(pbData, ushLen);
	pbData[ushLen] = 0x00;

	switch(*(u_short*)(pbData + 12))
	{
	case S_CRC_SIZE:
		std::cout << "S_CRC_SIZE received." << std::endl;
		std::cout << "CRC Package size : " << *(u_short*)(pbData + 8) << std::endl;
		m_uiCrcSize = *(u_short*)(pbData + 8);
		break;

	case S_GET_SERVER_TIME:
		std::cout << "S_GET_SERVER_TIME received." << std::endl;
		ostrStream << "Server Time is " << *(u_short*)(pbData + 16) /*Year*/ << "-" << *(u_short*)(pbData + 18) /*Month*/ << "-" << *(u_short*)(pbData + 22) /*Day*/ << " " << *(u_short*)(pbData + 24) /*Hour*/ << ":" << *(u_short*)(pbData + 26) /*Minute*/ << ":" << *(u_short*)(pbData + 28) /*Seconds*/;
		SetConsoleTitleA(ostrStream.str().c_str());
		break;

	case S_UPDATE_CRYPT_USERKEY:
		std::cout << "S_UPDATE_CRYPT_USERKEY received." << std::endl;

		//Save new short key
		memcpy(pbShortKey, (pbData + 16), 16);

		//Prepare packet
		vSendVector.resize(vSendVector.size() + 16);
		memcpy(&vSendVector[vSendVector.size() - 16], pbCryptUserKeyPacket, 16);

		vSendVector.resize(vSendVector.size() + 16);
		memcpy(&vSendVector[vSendVector.size() - 16], pbShortKey, 16);

		vSendVector[8] = m_uiSendCounter;

		SendEncryptedPacket(vSendVector.data(), vSendVector.size());
		std::cout << "C_UPDATE_CRYPT_USERKEY sent." << std::endl;

		//Update new crypt key
		SEEDMakeCryptKey(pbShortKey, m_pbLongKey);

		break;

	case S_CHECKSUM_PACKAGE:
		std::cout << "CRC Package received." << std::endl;

		m_bIgnorePackets = TRUE;
		break;

	case S_CHECKSUM_PACKAGE_FINISHED:
		std::cout << "S_CHECKSUM_PACKAGE_FINISHED received." << std::endl;

		pbCRCSuccessPacket[8] = m_uiSendCounter;

		vSendVector.resize(vSendVector.size() + 32);
		memcpy(&vSendVector[vSendVector.size() - 32], pbCRCSuccessPacket, 32);

		SendEncryptedPacket(vSendVector.data(), 32);
		std::cout << "C_CRC_CHECK_SUCCESS sent." << std::endl;
		m_bIgnorePackets = FALSE;
		break;

	case S_CHECKSUM_DO_SCAN:
		std::cout << "S_CHECKSUM_DO_SCAN received." << std::endl;

		pbCRCSuccessPacket[8] = m_uiSendCounter;

		vSendVector.resize(vSendVector.size() + 16);
		memcpy(&vSendVector[vSendVector.size() - 16], pbCRCSuccessPacket, 16);

		SendEncryptedPacket(vSendVector.data(), 32);
		std::cout << "C_CRC_CHECK_SUCCESS sent." << std::endl;
		break;

	case S_UNK01:
		std::cout << "S_UNK01 received." << std::endl;
		break;

	case S_DEBUGGER_PATTERN:
		std::cout << "S_DEBUGGER_PATTERN received." << std::endl;
		break;

	case S_HACK_PATTERN_START:
		std::cout << "S_HACK_PATTERN_START received." << std::endl;
		break;

	case S_HACK_PATTERN_PACKAGE:
		std::cout << "Hack Pattern Package received." << std::endl;
		m_bIgnorePackets = TRUE;
		break;

	case S_HACK_PATTERN_PACKAGE_FINISHED:
		std::cout << "Hack Pattern Check Package finished." << std::endl;
		m_bIgnorePackets = FALSE;
		break;

	case S_MEM_PATTERN_START:
		std::cout << "S_MEM_PATTERN_START received." << std::endl;
		break;

	case S_MEM_PATTERN_IDENTIFIER:
		std::cout << "S_MEM_PATTERN_IDENTIFIER received." << std::endl;

		break;

	case S_CHECK_API_HOOKS:
		{
			HANDLE hS4Client = 0;
			char szHGWSK[224] = {0};
			char szFullPath[MAX_PATH] = {0};
			std::string commandLine;
			std::cout << "S_CHECK_API_HOOKS received." << std::endl;
			byte size = *(BYTE*)(pbData + 17);

			memcpy(&szHGWSK[0], (pbData + 18), size);
			commandLine += "S4Client.exe ";
			commandLine += "-HGWSK ";
			commandLine += szHGWSK;
			commandLine += " -rc:eu -lac:eng -auth_server_ip:195.122.162.94";
			
			//Start game
			GetFullPathNameA("S4Client.exe", MAX_PATH, szFullPath, nullptr);
			hS4Client = CreateS4Client(szFullPath, commandLine);

			if(hS4Client)
			{
				GetFullPathNameA("XFB.dll", MAX_PATH, szFullPath, nullptr);
				DetourContinueProcessWithDllA(hS4Client, szFullPath);
			}

			

			pbXtrapInitedPacket[8] = m_uiSendCounter;

			vSendVector.resize(vSendVector.size() + 16);
			memcpy(&vSendVector[vSendVector.size() - 16], pbXtrapInitedPacket, 16);

			SendEncryptedPacket(vSendVector.data(), 16);
			std::cout << "C_INITED_XTRAP_ENGINE sent." << std::endl;

			Sleep(2000);

			vSendVector.clear();

			//pbUserInfoPacket[8] = m_uiSendCounter;

			//SendEncryptedPacket(vSendVector.data(), vSendVector.size());
			//std::cout << "C_NOTIFY_USERINFO sent." << std::endl;
		}
		break;

	case S_UNK03:
		std::cout << "S_UNK03 received." << std::endl;
		break;

	case S_KEEP_ALIVE:
		std::cout << "S_KEEP_ALIVE received." << std::endl;
		break;

	case S_CS_AUTH:
		std::cout << "S_CS_AUTH received." << std::endl;

		pbAuthPacket[8] = m_uiSendCounter;
		
		vSendVector.resize(vSendVector.size() + 16);
		memcpy(&vSendVector[vSendVector.size() - 16], pbAuthPacket, 16);

		SendEncryptedPacket(vSendVector.data(), 16);
		std::cout << "C_CS_AUTH sent." << std::endl;

		break;

	case S_XTRAP_STEP1:
		//MAX SIZE =>0x80

		std::cout << "S_XTRAP_STEP1 received." << std::endl;

		for(int i = 0; i < 128; ++i)
		{
			if(iColLen <= 10)
			{
				sprintf_s(szBuffer, "%02X ", *(BYTE*)(pbData + 16 + i));
				ostrStream.write(szBuffer, 3);

				++iColLen;
			}
			else
			{
				ostrStream << std::endl;
				iColLen = 0;
			}
		}

		std::cout << "=====preKeepAlive=====" << std::endl;
		std::cout << ostrStream.str().c_str() << std::endl;

		pbXtrapCmdPacket[8] = m_uiSendCounter;

		vSendVector.resize(vSendVector.size() + 160);
		memcpy(&vSendVector[vSendVector.size() - 160], pbXtrapCmdPacket, 160);

		SendEncryptedPacket(vSendVector.data(), vSendVector.size());
		std::cout << "C_XTRAP_CMD sent." << std::endl;

		break;

	default:

		//if(m_bIgnorePackets)
		//	return;

		std::cout << "Unknown Packet received." << std::endl;

		for(int i = 0; i < ushLen; ++i)
		{
			sprintf_s(szBuffer, 10, "%02X ", *(BYTE*)(pbData + i));
			ostrStream.write(szBuffer, 3);
		}
		CLog::CLog("Packet : ");
		CLog::CLog(ostrStream.str());
	}

}

bool CHGWC::SendEncryptedPacket(PBYTE pbData, unsigned short ushLen)
{
	unsigned int uiCounter = 0;
	bool bRet = TRUE;

	while(uiCounter < ushLen)
	{
		SEEDEncryptDataBlock(m_pbLongKey, pbData);
		uiCounter += 16;
		pbData += 16;
	}

	pbData -= ushLen;

	if(send(m_sClientSocket, (char*)pbData, ushLen, 0) != ushLen)
		bRet = FALSE;
	
	++m_uiSendCounter;

	return bRet;
}

void CHGWC::DecryptPacket(PBYTE pbData, unsigned short ushLen)
{
	unsigned int uiCounter = 0;

	while(uiCounter < ushLen)
	{
		SEEDDecryptDataBlock(m_pbLongKey, pbData);
		uiCounter += 16;
		pbData += 16;
	}

	pbData -= ushLen;

}

void CHGWC::EncryptPacket(PBYTE pbData, unsigned short ushLen)
{
	unsigned int uiCounter = 0;

	while(uiCounter < ushLen)
	{
		SEEDEncryptDataBlock(m_pbLongKey, pbData);
		uiCounter += 16;
		pbData += 16;
	}

	pbData -= ushLen;
}