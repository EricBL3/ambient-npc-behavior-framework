#include "FrameworkLogger.h"

#include <chrono>
#include <stdexcept>
#include <fstream>
#include <iomanip>

using namespace AmbientCharacterBehavior;

FrameworkLogger::FrameworkLogger(const std::string &log_file_path)
{
    log_level = FrameworkLogLevel::INFO;

    if (!log_file_path.empty())
    {
        log_file = make_unique<std::ofstream>(log_file_path, std::ios::out | std::ios::app);

        if (!log_file->is_open())
        {
            throw std::runtime_error("FrameworkLogger: Failed to open log file: " + log_file_path);
        }

        is_logging_enabled = true;
    }
    else
    {
        is_logging_enabled = false;
    }
}

bool FrameworkLogger::Initialize(const std::string &log_file_path, FrameworkLogLevel log_level)
{
    if (!log_file_path.empty())
    {
        log_file = make_unique<std::ofstream>(log_file_path, std::ios::out | std::ios::app);

        if (!log_file->is_open())
        {
            throw std::runtime_error("FrameworkLogger: Failed to open log file: " + log_file_path);
        }

        is_logging_enabled = true;

        SetLogLevel(log_level);
    }
    else
    {
        is_logging_enabled = false;
    }

    return is_logging_enabled;
}

void FrameworkLogger::WriteLog(FrameworkLogLevel level, const std::string &message, const std::string &component_name)
{
    if (!is_logging_enabled || level < log_level || !log_file || !log_file->is_open())
    {
        return;
    }

    std::string level_str;
    switch (level)
    {
        case FrameworkLogLevel::INFO:
            level_str = "INFO";
            break;
        case FrameworkLogLevel::WARNING:
            level_str = "WARNING";
            break;
        case FrameworkLogLevel::ERROR:
            level_str = "ERROR";
            break;
    }

    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);

    *log_file << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] ";
    *log_file << "[" << level_str << "] [" << component_name << "] " << message << std::endl;

    log_file->flush();
}
