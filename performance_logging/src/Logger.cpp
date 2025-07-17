#include "../include/Logger.hpp"
#include <iostream>
#include <ctime>

Logger::Logger() {
    logFile.open("logs/performance.log", std::ios::app);
}

Logger::~Logger() {
    if (logFile.is_open()) logFile.close();
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::log(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::time_t now = std::time(nullptr);
    logFile << "[" << std::ctime(&now) << "]: " << message << std::endl;
}
