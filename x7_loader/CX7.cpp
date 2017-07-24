#include "CX7.h"
#include "Breakpoint.h"
#include "Memory.h"

#include <Windows.h>

#include <string>
#include <vector>
#include <sstream>
#include <fstream>

#include "detours.h"

CX7::CX7(DWORD dwHookAddress, PVOID lpHookProc, bool bImport, PBYTE &rpbTrampoline, DWORD dwPatchAddr)
{
	HANDLE xtrapDll = 0;
	//if(!dwHookAddress || !lpHookProc || !dwPatchAddr)
	//	throw std::runtime_error("Invalid addresses!");
 
	while(!xtrapDll)
	{
		xtrapDll = GetModuleHandleA("XTrapVa.dll");
		Sleep(100);
	}

	CreateDirectoryA("x7", NULL);

	//Memory::GetInstance().InitWithAddress((void*)0x4044DAE4);

	//Memory::GetInstance().DuplicatePage((void*)dwHookAddress, 0x1000);
	//Memory::GetInstance().DuplicatePage((void*)dwPatchAddr, 0x1000);

	rpbTrampoline = DetourFunction((PBYTE)dwHookAddress, (PBYTE)lpHookProc);
	DetourRemove(rpbTrampoline, (PBYTE)lpHookProc);
	/*memset((void*)dwPatchAddr, 0x90, 5);*/
	
		
	Breakpoint = new CBreakpoint();

	Breakpoint->AddBreakpoint(dwHookAddress, (DWORD)lpHookProc, 0);
	Breakpoint->AddBreakpoint(dwPatchAddr, (dwPatchAddr + 0x5), 0);

	Breakpoint->SetupThread(0);

	m_bImport = bImport;
	m_bDoImport = false;
	m_uiCounter = 0;
	m_vstrModifiedX7Files.push_back("Dummy");
	m_vstrOriginalX7Files.push_back("Dummy");
}

//-----------------------------------------------------------------------------------------------------

CX7::~CX7()
{
	
}

//-----------------------------------------------------------------------------------------------------

std::string CX7::GetModifiedResourceFile(unsigned int uiIndex) const
{
	try
	{
		return m_vstrModifiedX7Files.at(uiIndex);	
	}
	catch(const std::out_of_range &rException)
	{
		MessageBoxA(NULL, rException.what(), "Exception occured!", MB_OK);
	}
}

//-----------------------------------------------------------------------------------------------------

void CX7::ResourceProc(const char *szX7Content)
{
	std::string strImportBuffer;

	char szPathBuffer[MAX_PATH];
    
	std::streampos streamPos;

	std::ifstream ifstrFile;
	std::ofstream ofstrFile;
	std::ostringstream ostrStream;

	++m_uiCounter;

	m_vstrOriginalX7Files.push_back(szX7Content);
	m_vstrModifiedX7Files.push_back("");

	//Import
	if(m_bImport)
	{
		/************************************************************************/
		/* Get original file                                                                     */
		/************************************************************************/
		GetCurrentDirectoryA(MAX_PATH, szPathBuffer);
		ostrStream << szPathBuffer << "\\x7\\" << m_uiCounter << "[].x7"; //  "%s\\x7\\%d[].x7"

		//Import file because of []
		if(GetFullPathNameA(ostrStream.str().c_str(), MAX_PATH, szPathBuffer, NULL))
		{
			//Open file
			ifstrFile.open(ostrStream.str().c_str(), std::ifstream::ate | std::ifstream::binary);

			if(ifstrFile.is_open())
			{
				/*Obtain file size*/
				streamPos = ifstrFile.tellg();
				ifstrFile.seekg(std::ifstream::beg);

				if(streamPos > 0)
				{
					//Read file
					strImportBuffer = std::string((std::istreambuf_iterator<char>(ifstrFile)), std::istreambuf_iterator<char>());
					
					if(strImportBuffer.length() == streamPos)
					{
						//Save file content into std::string array
						m_vstrModifiedX7Files.pop_back();
						m_vstrModifiedX7Files.push_back(strImportBuffer.c_str() + 5);

						MessageBoxA(NULL, szPathBuffer, "Imported - x7", NULL);

						m_bDoImport = true;
					}
					else
					{
						m_bDoImport = false;
					}
				}
				else
				{
					m_bDoImport = false;
				}
			}
			else
			{
				m_bDoImport = false;
			}

			//Cleanup
			ifstrFile.close();
		}
		else
		{
			m_bDoImport = false;
		}
	}
	else
	{
		try
		{
			//Create new x7 file 
			GetCurrentDirectoryA(MAX_PATH, szPathBuffer);
			ostrStream << szPathBuffer << "\\x7\\" << m_uiCounter << ".x7"; //  "%s\\x7\\%d[].x7"

			ofstrFile.open(ostrStream.str().c_str(), std::ofstream::binary);
			ofstrFile.seekp(std::ios::end);

			if(ofstrFile.is_open())
			{
				ofstrFile.write((m_vstrOriginalX7Files.at(m_uiCounter).c_str()), m_vstrOriginalX7Files.at(m_uiCounter).length());
			}

			ifstrFile.close();
		}
		catch(const std::out_of_range &rException)
		{
			MessageBoxA(NULL, rException.what(), "Exception occured!", NULL);
		}
		catch(...)
		{
			MessageBoxA(NULL, "Unknown exception occured!", "?", NULL);
		}
		
	}

}

//-----------------------------------------------------------------------------------------------------

bool CX7::CanImport()
{
	return m_bDoImport;
}

//-----------------------------------------------------------------------------------------------------

unsigned int CX7::GetCounter()
{
	return m_uiCounter;
}

//-----------------------------------------------------------------------------------------------------

unsigned int CX7::GetResourceFileSize(UINT uiIndex)
{
	return m_vstrModifiedX7Files[uiIndex].length();
}

//-----------------------------------------------------------------------------------------------------