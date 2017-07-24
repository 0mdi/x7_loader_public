#include "CThemida.h"
#include "detours.h"

#include <Windows.h>
#include <string>
#include <Psapi.h>

typedef void (WINAPI *tGetStartupInfoA)(LPSTARTUPINFOA);
tGetStartupInfoA oGetStartupInfoA;

void WINAPI hkGetStartupInfoA(LPSTARTUPINFOA lpStartupInfo)
{
	DWORD dwCalledFrom = 0;
	MODULEINFO moduleInfo = {0};

	std::string strProcessName;
	strProcessName.resize(strProcessName.size() + MAX_PATH);

	//Get return address
	_asm
	{
		push eax
		mov eax, [ebp + 4]
		mov [dwCalledFrom], eax
	}

	if(!GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(nullptr), &moduleInfo, sizeof(MODULEINFO)))
		MessageBoxA(NULL, "GetModuleInformation failed!", "Error!", NULL);

	if(dwCalledFrom > (DWORD)moduleInfo.EntryPoint && dwCalledFrom < ((DWORD)moduleInfo.EntryPoint) + moduleInfo.SizeOfImage)
	{
		if(!GetModuleFileNameA(NULL, (char*)strProcessName.data(), MAX_PATH))
		{
			MessageBoxA(NULL, "GetModuleFileNameA failed!", "Error!", NULL);
			CThemida::Singleton()->SetClientType(ClientType::UnknownProcess);
		}
		else 
		{

			if(strProcessName.find("HGWC") != std::string::npos)
				CThemida::Singleton()->SetClientType(ClientType::HGWC);
			else if(strProcessName.find("Xtrap") != std::string::npos)
				CThemida::Singleton()->SetClientType(ClientType::XTrapXT);
			else if(strProcessName.find("S4Client") != std::string::npos)
				CThemida::Singleton()->SetClientType(ClientType::S4Client);
			else
				CThemida::Singleton()->SetClientType(ClientType::UnknownProcess);
		}

		CThemida::Singleton()->CallPatchFunc();
	}

	return oGetStartupInfoA(lpStartupInfo);
}

CThemida *CThemida::m_pSingleton = nullptr;

CThemida* CThemida::Singleton()
{
	if(!m_pSingleton)
		m_pSingleton = new CThemida();

	return m_pSingleton;
}

CThemida::CThemida()
{
	m_pPatchFunc = nullptr;
}

CThemida::~CThemida()
{

}

void CThemida::CallPatchFunc()
{
	if(m_pPatchFunc)
		m_pPatchFunc();
}

void CThemida::SetUp(const tPatchFunc &pPatchFunc)
{
	if(!pPatchFunc)
	{
		MessageBoxA(NULL, "Invalid patch function!", "Error!", NULL);
		return;
	}

	m_pPatchFunc = pPatchFunc;

	oGetStartupInfoA = (tGetStartupInfoA)DetourFunction((PBYTE)GetStartupInfoA, (PBYTE)hkGetStartupInfoA);
}

ClientType CThemida::GetClientType()
{
	return m_ClientType;
}

void CThemida::SetClientType(ClientType clientType)
{
	m_ClientType = clientType;
}