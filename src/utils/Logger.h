#ifndef SMARTDATAVISUALISATION_LOGGER_H
#define SMARTDATAVISUALISATION_LOGGER_H

#include <string>

namespace sdv::utils {

enum class LogLevel {
    Info,
    Warning,
    Error
};

void log(LogLevel level,
         const std::string& message);

void info(const std::string& message);

void warning(const std::string& message);

void error(const std::string& message);

} // namespace sdv::utils

#endif // SMARTDATAVISUALISATION_LOGGER_H
