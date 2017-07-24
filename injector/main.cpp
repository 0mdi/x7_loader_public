#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>
#include "detours.h"
#include "resource.h"
#include "CHGWC.h"

bool CreateResourceAsTempFile(WORD wResource, WORD wType, std::string &strPathToNewFile)
{
	char szTmpFullPath[MAX_PATH] = {0};
	HANDLE hTempFile = NULL;
	HRSRC hResInfo = NULL;
	HGLOBAL glbTempFile = NULL;
	void *pbResourceData = NULL;
	DWORD dwBytesWrote;


	//Temp path
	GetTempPathA(MAX_PATH, szTmpFullPath);
	GetTempFileNameA(szTmpFullPath, "NN", NULL, szTmpFullPath);

	//Create Tempfile
	hTempFile = CreateFileA("C:\\odi.dll", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hTempFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	//Get Resource information
	hResInfo = FindResourceA(NULL, MAKEINTRESOURCEA(wResource), MAKEINTRESOURCEA(10));

	if(!hResInfo)
	{
		CloseHandle(hTempFile);
		return 0;
	}

	//Get Gloabl resource file
	glbTempFile = LoadResource(NULL, hResInfo);

	if(!glbTempFile)
	{
		CloseHandle(hTempFile);
		return 0;
	}

	//Get resource data
	pbResourceData = (void*)malloc(SizeofResource(NULL, hResInfo));
	pbResourceData = LockResource(glbTempFile);
	UnlockResource(glbTempFile);

	if(!pbResourceData)
	{
		CloseHandle(hTempFile);
		return 0;
	}

	//Write data into temp file
	SetFilePointer(hTempFile, 0, 0, FILE_END);

	if(!WriteFile(hTempFile, pbResourceData, SizeofResource(NULL, hResInfo), &dwBytesWrote, NULL))
	{
		CloseHandle(hTempFile);
		return 0;
	}

	//Set path
	strPathToNewFile = "C:\\odi.dll";
	CloseHandle(hTempFile);

	return 1;
}

HANDLE GetProcessHandle(wchar_t *process_name, DWORD dwAccess)
{
HANDLE hProcessSnap;
HANDLE hProcess;
PROCESSENTRY32 pe32;


hProcessSnap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);

if(hProcessSnap==INVALID_HANDLE_VALUE)
  {
    return INVALID_HANDLE_VALUE;
  }

pe32.dwSize=sizeof(PROCESSENTRY32);

if(!Process32First(hProcessSnap,&pe32))
  {
      return INVALID_HANDLE_VALUE;
  }

do
  {
    if(wcscmp(pe32.szExeFile,process_name)==0)
        return OpenProcess(dwAccess,0,pe32.th32ProcessID);

  }while(Process32Next(hProcessSnap,&pe32));

  return 0;
}

/************************************************************************/
/* Main                                                                     */
/************************************************************************/
int main()
{
	HANDLE s4Process = 0;

	//Setconsole title
	SetConsoleTitleA("x7Loader - VIP EDITION");

	std::cout<<"######################################"<<std::endl;
	std::cout<<"#Author   : Omdihar                  #"<<std::endl;
	std::cout<<"#Name     : x7Loader                 #"<<std::endl;
	std::cout<<"#Version  : OPEN SOURCE              #"<<std::endl;
	std::cout << "#Supports : Vista and above        #" << std::endl;
	std::cout<<"#Website  : https://omdischeats.com/ #"<<std::endl;
	std::cout<<"######################################"<<std::endl;

	std::cout << "Please start S4 now!" << std::endl;

	while(!s4Process)
	{
		s4Process = GetProcessHandle(L"S4Client.exe", PROCESS_ALL_ACCESS);
		Sleep(100);
	}

	std::string pathToDll;


	CreateResourceAsTempFile(IDR_RCDATA1, 10, pathToDll);

	DetourContinueProcessWithDllA(s4Process, pathToDll.c_str());
}