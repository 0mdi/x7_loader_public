#pragma once

#include <Windows.h>
#include <fstream>
#include <string>

class CLog
{
public:
	CLog(std::string strText);
	CLog();
private:
	static std::ofstream ofstrLogFile;
};