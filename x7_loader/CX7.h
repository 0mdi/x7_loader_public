#pragma once
#include <Windows.h>
#include <string>

#include <vector>


class CX7
{
public:
	CX7(DWORD dwHookAddress, PVOID lpHookProc, bool bImport, PBYTE &rpbTrampoline, DWORD dwPatchAddr);
	~CX7();

	//-----------------------------------------------------------------------------------------------------

	std::string GetModifiedResourceFile(unsigned int uiIndex) const;

	//-----------------------------------------------------------------------------------------------------

	void ResourceProc(const char *szX7Content);

	//-----------------------------------------------------------------------------------------------------

	bool CanImport();

	//-----------------------------------------------------------------------------------------------------

	unsigned int GetCounter();

	//-----------------------------------------------------------------------------------------------------

	unsigned int GetResourceFileSize(unsigned int uiIndex);

	//-----------------------------------------------------------------------------------------------------

private:

	//Original x7Files
	std::vector<std::string> m_vstrOriginalX7Files;

	//Modified x7Files
	std::vector<std::string> m_vstrModifiedX7Files;

	//true  : Import
	//false : Export
	bool m_bImport;


	bool m_bDoImport;

	//Counter
	unsigned int m_uiCounter;
};
