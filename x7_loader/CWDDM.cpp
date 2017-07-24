#include "CWDDM.h"

#include <Windows.h>
#include "detours.h"
#include <Psapi.h>

#pragma comment(lib, "Psapi")

CWDDM *CWDDM::m_Instance = nullptr;
LPDIRECT3DDEVICE9 g_pDevice = nullptr;

IDirect3DDevice9* CWDDM::m_pDevice = NULL;

VOID (__stdcall* pInterceptAdapter)() = NULL;
HRESULT(__stdcall* pAdpFlush)(ULONG) = NULL;

CWDDM::CWDDM()
{
	m_pDevice = NULL;
}

CWDDM::~CWDDM()
{

}

//Singleton
CWDDM *CWDDM::getInstance()
{
	if(m_Instance == nullptr)
		m_Instance = new CWDDM();

	return m_Instance;
}


HRESULT __stdcall myAdpFlush(ULONG pAdapterDev)//Nvidia cards on some games do not call present and go straight to flush.
{
	CWDDM::getInstance()->m_pRenderCallBack(g_pDevice);
	HRESULT result = pAdpFlush(pAdapterDev);
	return result;
}


VOID RedirectAdapter(ADAPTER_STRUCT* pAdpStruct)
{
	if(IsBadCodePtr((FARPROC)pAdpStruct))
		return;

	g_pDevice = pAdpStruct->pDev;


	pAdpFlush = (HRESULT(__stdcall*)(ULONG))DetourFunction((PBYTE)pAdpStruct->AdapterFuncs[AdpFlush], (PBYTE)myAdpFlush);
	/*pAdpPresent = (HRESULT(__stdcall*)(HANDLE, D3DDDIARG_PRESENT*))DetourFunction((PBYTE)pAdpStruct->AdapterFuncs[AdpPresent], (PBYTE)myAdpPresent);*/
}

__declspec(naked)VOID __stdcall myInterceptAdapter()
{
	__asm
	{
		pushad;
		push ecx;
		call RedirectAdapter;
		push myInterceptAdapter;
		push pInterceptAdapter;
		call DetourRemove;
		pop ecx;
		popad;
		jmp [pInterceptAdapter];
		retn;
	}
}

bool CWDDM::ReadSignature(DWORD dwAddress, BYTE *pbSignature, int iLenght)
{
	for(INT j = 0; j < iLenght; ++j)
	{
		BYTE pbCur = *(BYTE*)(dwAddress + j);

		if(pbCur == pbSignature[j] || pbSignature[j] == NULL)
			continue;
		else
			return FALSE;

	}	
	return TRUE;
}


void CWDDM::init(HMODULE hD3d9, tRenderCallBack pRenderCallBack)
{
	m_pRenderCallBack = pRenderCallBack;

	MODULEINFO pModuleInfo;

	if(GetModuleInformation(GetCurrentProcess(), hD3d9, &pModuleInfo, sizeof(MODULEINFO)))
	{
		for(DWORD j = (DWORD)pModuleInfo.lpBaseOfDll; j < ((DWORD)pModuleInfo.lpBaseOfDll + pModuleInfo.SizeOfImage); ++j)
		{
			if(ReadSignature(j, (BYTE*)"\x8B\xFF\x55\x8B\xEC\x83\xEC\x08\x53\x8B\x5D\x08\x56\x57\x8B\x7D\x0C\x8B\xF1\x57\x53\x8D\x8E", 23))
			{
				pInterceptAdapter = (VOID(__stdcall*)())DetourFunction((PBYTE)j, (PBYTE)myInterceptAdapter);
			}
		}
	}
}