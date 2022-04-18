#pragma once

#include <windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <sstream>

static std::string getExePath()
{
    TCHAR buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    return std::string(buffer);
}

static std::string getWorkingDir()
{
	namespace fs = std::filesystem;
    std::stringstream ss;
    ss << fs::current_path();
    return ss.str();
}

static std::string getFileDir(const char* fileMacro)
{
    std::string sPath(fileMacro);
    std::string dirPath = sPath.substr(0, sPath.rfind("\\"));
    return dirPath;
}
