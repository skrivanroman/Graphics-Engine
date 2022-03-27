#include "Logger.hpp"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

Logger::Logger(std::ostream& outputStream, LogLevel level,bool logColor, bool logType, bool logTime)
    : outputStream(outputStream), level(level), logColor(logColor), logType(logType), logTime(logTime)
{}

Logger::~Logger()
{
    if (logColor)
		outputStream << RESET;
}

void Logger::info(const std::string& message)
{
    if (level > LogLevel::Info) return;

    if (logColor) outputStream << WHITE;
    outputStream << '[';
    if (logType) outputStream << INFO; 
    
    log(message);
}

void Logger::warning(const std::string& message)
{
    if (level > LogLevel::Warning) return;

    if (logColor) outputStream << GREEN;
    outputStream << '[';
    if (logType) outputStream << WARNING;
    
    log(message);
}

void Logger::error(const std::string& message)
{
    if (level > LogLevel::Error) return;

    if (logColor) outputStream << YELLOW;
    outputStream << '[';
    if (logType) outputStream << ERROR;
    
    log(message);
}

void Logger::critical(const std::string& message)
{
    if (level > LogLevel::Critical) return;

    if (logColor) outputStream << RED;
    outputStream << '[';
    if (logType) outputStream << CRITICAL;
    
    log(message);
}

void Logger::newLine()
{
    outputStream << '\n';
}

void Logger::log(const std::string& message)
{
    std::string timestamp = "";
    if (logTime)
        timestamp = getCurrentTime();

    timestamp += "]: ";

    outputStream << timestamp << message << '\n';
}

std::string Logger::getCurrentTime() const
{

    time_t now = std::time(nullptr);
    tm time;
    localtime_s(&time, &now);

    std::ostringstream oss;
    oss << std::put_time(&time, "%H:%M:%S");
    return oss.str();
}


void Logger::glfwCallback(int error, const char* message)
{
    LOG_ERROR(message);
}
