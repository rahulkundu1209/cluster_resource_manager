#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <mutex>
#include <string>

class Logger {
public:
    static Logger& getInstance();
    void log(const std::string& message);

private:
    Logger();
    ~Logger();
    std::ofstream logFile;
    std::mutex logMutex;
};

#endif
