#pragma once
#include <Windows.h>

typedef void (__stdcall *tPatchFunc)();

enum ClientType
{
	XTrapXT,
	HGWC,
	S4Client,
	UnknownProcess
};

class CThemida
{
public:

	static CThemida* Singleton();

	void SetUp(const tPatchFunc &pPatchFunc);
	void CallPatchFunc();

	ClientType GetClientType();
	void SetClientType(ClientType clientType);

protected:
	CThemida();
	~CThemida();

private:

	tPatchFunc m_pPatchFunc;
	ClientType m_ClientType;

	static CThemida *m_pSingleton;
};
