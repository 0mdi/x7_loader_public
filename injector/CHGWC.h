#pragma once
#include <Windows.h>
#include <string>

#include "SEED_impl.h"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "SEED_impl.lib")

class CHGWC
{
public:
	static CHGWC* Singleton();

	void Init(std::string strIP, unsigned int ushPort);
	void HandlePacketEx();
	void HandlePacket(PBYTE pbData, unsigned short ushLen);

	bool SendEncryptedPacket(PBYTE pbData, unsigned short ushLen);
	void DecryptPacket(PBYTE pbData, unsigned short ushLen);
	void EncryptPacket(PBYTE pbData, unsigned short ushLen);

protected:
	CHGWC();
	~CHGWC();

	void PrepareConnection();

private:

	SOCKET m_sClientSocket;

	BYTE m_pbLongKey[128];

	bool m_bIgnorePackets;

	unsigned int m_uiSendCounter;
	unsigned int m_uiCrcSize;

	HANDLE m_hPipe;

	static CHGWC *m_pSingleton;
};