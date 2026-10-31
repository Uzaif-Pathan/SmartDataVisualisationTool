#include "ValidationResult.h"

namespace sdv {

std::string severityToString(Severity s) {
    switch (s) {
        case Severity::Info:    return "INFO";
        case Severity::Warning: return "WARNING";
        case Severity::Error:   return "ERROR";
    }
    return "UNKNOWN";
}

} // namespace sdv
