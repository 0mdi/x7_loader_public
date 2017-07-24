#include <Windows.h>
#include <d3d9.h>
//#include <d3dx9.h>
#include <thread>

#include "detours.h"


#include "CX7.h"
#include "resource.h"
#include "Breakpoint.h"
//#include "ImportImage.h"
#include "CWDDM.h"
#include "CThemida.h"

//#pragma comment(lib, "d3d9.lib")
//#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "detours")


//Instance of CX7
CX7 *x7;


HINSTANCE hInstance = NULL;

//Prototype of the original function
typedef bool (__thiscall *tLoadX7)(void*, int, char*, int, int);
tLoadX7 pLoadX7 = NULL;

typedef BOOL (WINAPI *tCreateProcessW)(LPCTSTR, LPTSTR, LPSECURITY_ATTRIBUTES,
									  LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID,
									  LPCTSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
tCreateProcessW		oCreateProcessW		= nullptr;

/************************************************************************/
/* Dialog for Import and Export       
 * */
/************************************************************************/
INT_PTR CALLBACK DlgProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	switch(uMsg)
	{

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTONIMPORT:
			EndDialog(hWndDlg, 1); // 0 = Export | 1 = Import
			break;
		case IDC_BUTTONEXPORT:
			EndDialog(hWndDlg, 0);
			break;
		}
		break;

	}

	return FALSE;
}

/************************************************************************/
/* GetImportExportDialog
 * \brief
 * false : Import
 * true  : Export
 * 
 * \param
 * hInstance : Current instance*/
/************************************************************************/
bool GetImportExportDialog(HINSTANCE hInstance)
{
	INT_PTR iptrReturn;

	iptrReturn = DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
	return (bool)iptrReturn;
}

BOOL WINAPI hkCreateProcessW(LPCTSTR lpApplicationName, LPTSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
							 LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags,
							 LPVOID lpEnvironment, LPCTSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo,
							 LPPROCESS_INFORMATION lpProcessInformation)
{
	BOOL bRet;

	bRet = oCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, CREATE_SUSPENDED,
						   lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

	DetourContinueProcessWithDllA(lpProcessInformation->hProcess, "C:\\odi.dll");

	ResumeThread(lpProcessInformation->hThread);
	ResumeThread(lpProcessInformation->hProcess);

	return bRet;
}

/************************************************************************/
/* x7Hook 
 * \brief
 * Hook function which exports and imports the x7 files
 * \param
 * pThis      :  Instance
 * pUnknown   :  Unknown
 * iUnknown1  :  Unknown
 * szX7Buffer :  Contains the x7 file
 * iSize      :  Size of the x7 file
 * iUnknown2  :  Unknown
 * */
/************************************************************************/
bool __fastcall x7Hook(void *pThis, void *pUnknown, int iUnknown1, char *szX7Buffer, int iSize, int iUnknown2)
{
	UINT uiCounter = 0;

	x7->ResourceProc(szX7Buffer);

	uiCounter = x7->GetCounter();

	if(x7->CanImport())
	{
		iSize = x7->GetResourceFileSize(uiCounter);

		return pLoadX7(pThis, iUnknown1, const_cast<char*>(x7->GetModifiedResourceFile(uiCounter).c_str()), iSize, iUnknown2);
	}

	return pLoadX7(pThis, iUnknown1, szX7Buffer, iSize, iUnknown2);
}

DWORD __stdcall patch(LPVOID)
{
	x7 = new CX7(0x00EB8190, (PVOID)x7Hook, (bool)GetImportExportDialog(hInstance), (PBYTE&)pLoadX7, 0x00EB8285);
	return 0;
}

/************************************************************************/
/* Dllmain                                                                     */
/************************************************************************/
BOOL WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		hInstance = (HINSTANCE)hDllHandle;
		CreateThread(0, 0, patch, nullptr, 0, nullptr);
	}

	return TRUE;
}