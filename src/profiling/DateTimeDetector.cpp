#include "DateTimeDetector.h"
#include <regex>

namespace sdv {

namespace {
bool matchesDateFormat(const std::string& s) {
    static const std::regex isoDate(R"(^\d{4}-\d{2}-\d{2}$)");
    static const std::regex dmy(R"(^\d{2}/\d{2}/\d{4}$)");
    static const std::regex isoDateTime(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$)");
    return std::regex_match(s, isoDate) ||
           std::regex_match(s, dmy) ||
           std::regex_match(s, isoDateTime);
}
} // namespace

double DateTimeDetector::dateLikeRatio(const DataColumn& column) const {
    size_t nonMissing = 0, dateLike = 0;
    for (size_t i = 0; i < column.size(); ++i) {
        const DataValue& v = column.at(i);
        if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
        ++nonMissing;
        if (matchesDateFormat(v.asString())) ++dateLike;
    }
    if (nonMissing == 0) return 0.0;
    return static_cast<double>(dateLike) / static_cast<double>(nonMissing);
}

bool DateTimeDetector::isLikelyDateTime(const DataColumn& column, double threshold) const {
    return dateLikeRatio(column) >= threshold;
}

} // namespace sdv
