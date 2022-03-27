#pragma once

#include <iostream>
#include <string>

#define LOGGING_ON 1

enum class LogLevel
{
    Info,
    Warning,
    Error,
    Critical
};

class Logger 
{
public:
    explicit Logger(
        std::ostream& outputStream, LogLevel level, bool logColor = false,
        bool logType = true, bool logTime = true
    );

    ~Logger();

    Logger(const Logger&) = delete;

    Logger& operator= (const Logger&) = delete;

    void info(const std::string& message);

    void warning(const std::string& message);

    void error(const std::string& message);

    void critical(const std::string& message);

    void newLine();

    static void glfwCallback(int error, const char* message);

private: 
    void log(const std::string& message);

    std::string getCurrentTime() const;

private:
    std::ostream& outputStream;
    LogLevel level;
    bool logColor, logType, logTime;
};


static Logger logger(std::cout, LogLevel::Info, true, true, true);

#if LOGGING_ON
    #define LOG_INFO(message) logger.info(message)
    #define LOG_WARNING(message) logger.warning(message)
    #define LOG_ERROR(message) logger.error(message)
    #define LOG_CRITICAL(message) logger.critical(message)
    #define LOG_NEW_LINE() logger.newLine()
	#define STR(message) std::to_string(message)
#else
    #define LOG_INFO(message) 
    #define LOG_WARNING(message) 
    #define LOG_ERROR(message) 
    #define LOG_CRITICAL(message) 
    #define LOG_NEW_LINE() 
	#define STR(message)
#endif

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      
#define RED     "\033[31m"      
#define GREEN   "\033[32m"      
#define YELLOW  "\033[33m"     
#define BLUE    "\033[34m"     
#define MAGENTA "\033[35m"      
#define CYAN    "\033[36m"      
#define WHITE   "\033[37m" 

#define INFO "INFO   " 
#define WARNING "WARN   " 
#define ERROR "ERROR  "
#define CRITICAL "CRITIC "