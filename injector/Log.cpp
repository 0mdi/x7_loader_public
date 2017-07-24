#include "Log.h"
#include <Windows.h>

std::ofstream CLog::ofstrLogFile;

CLog::CLog(std::string strText)
{
	if(!ofstrLogFile.is_open())
		ofstrLogFile.open("C:\\HGWC.log", std::ofstream::binary | std::ofstream::ate);

	ofstrLogFile << strText << "\n";
	ofstrLogFile.flush();
}