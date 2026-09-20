#include "Logger.h"

#include <iostream>

namespace sdv::utils {

void log(LogLevel level,
         const std::string& message) {
    switch (level) {
        case LogLevel::Info:
            std::cout << "[INFO] " << message << '\n';
            break;

        case LogLevel::Warning:
            std::cerr << "[WARNING] " << message << '\n';
            break;

        case LogLevel::Error:
            std::cerr << "[ERROR] " << message << '\n';
            break;
    }
}

void info(const std::string& message) {
    log(LogLevel::Info, message);
}

void warning(const std::string& message) {
    log(LogLevel::Warning, message);
}

void error(const std::string& message) {
    log(LogLevel::Error, message);
}

} // namespace sdv::utils
