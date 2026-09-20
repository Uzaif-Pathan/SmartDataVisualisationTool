#include "NumericAnalyzer.h"
#include <cstdlib>
#include <cctype>

namespace sdv {

namespace {
bool looksNumeric(const std::string& raw) {
    size_t start = 0, end = raw.size();
    while (start < end && std::isspace(static_cast<unsigned char>(raw[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(raw[end - 1]))) --end;
    std::string s = raw.substr(start, end - start);
    if (s.empty()) return false;
    char* endPtr = nullptr;
    std::strtod(s.c_str(), &endPtr);
    return endPtr != s.c_str() && *endPtr == '\0';
}
} // namespace

double NumericAnalyzer::numericLikeRatio(const DataColumn& column) const {
    size_t nonMissing = 0, numeric = 0;
    for (size_t i = 0; i < column.size(); ++i) {
        const DataValue& v = column.at(i);
        if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
        ++nonMissing;
        if (v.isNumeric() || looksNumeric(v.asString())) ++numeric;
    }
    if (nonMissing == 0) return 0.0;
    return static_cast<double>(numeric) / static_cast<double>(nonMissing);
}

bool NumericAnalyzer::isLikelyNumeric(const DataColumn& column, double threshold) const {
    return numericLikeRatio(column) >= threshold;
}

} // namespace sdv
