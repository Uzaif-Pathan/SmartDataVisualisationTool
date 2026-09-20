#include "ValidationUtils.h"
#include <cctype>
#include <cstdlib>
#include <regex>

namespace sdv {
namespace validation_utils {

std::string trim(const std::string& text) {
    size_t start = 0, end = text.size();
    while (start < end && std::isspace(static_cast<unsigned char>(text[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1]))) --end;
    return text.substr(start, end - start);
}

bool isNumericLiteral(const std::string& text) {
    std::string t = trim(text);
    if (t.empty()) return false;
    char* endPtr = nullptr;
    std::strtod(t.c_str(), &endPtr);
    return endPtr != t.c_str() && *endPtr == '\0';
}

bool isDateLiteral(const std::string& text) {
    static const std::regex isoDate(R"(^\d{4}-\d{2}-\d{2}$)");
    static const std::regex dmy(R"(^\d{2}/\d{2}/\d{4}$)");
    static const std::regex isoDateTime(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$)");

    std::string t = trim(text);
    if (t.empty()) return false;
    return std::regex_match(t, isoDate) ||
           std::regex_match(t, dmy) ||
           std::regex_match(t, isoDateTime);
}

} // namespace validation_utils
} // namespace sdv
