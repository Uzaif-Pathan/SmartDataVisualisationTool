#include "CardinalityAnalyzer.h"
#include <unordered_set>

namespace sdv {

CardinalityInfo CardinalityAnalyzer::analyze(const DataColumn& column) const {
    CardinalityInfo info;
    std::unordered_set<std::string> seen;

    for (size_t i = 0; i < column.size(); ++i) {
        const DataValue& v = column.at(i);
        if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
        ++info.nonMissingCount;
        seen.insert(v.asString());
    }

    info.uniqueCount = seen.size();
    if (info.nonMissingCount > 0) {
        info.uniqueRatio =
            static_cast<double>(info.uniqueCount) / static_cast<double>(info.nonMissingCount);
    }
    return info;
}

} // namespace sdv
