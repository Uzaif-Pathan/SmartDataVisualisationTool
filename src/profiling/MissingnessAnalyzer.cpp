#include "MissingnessAnalyzer.h"

namespace sdv {

MissingnessInfo MissingnessAnalyzer::analyze(const DataColumn& column) const {
    MissingnessInfo info;
    info.totalCount = column.size();

    for (size_t i = 0; i < column.size(); ++i) {
        const DataValue& v = column.at(i);
        if (v.isMissing() || (v.isString() && v.asString().empty())) {
            ++info.missingCount;
        }
    }

    if (info.totalCount > 0) {
        info.missingPercent =
            100.0 * static_cast<double>(info.missingCount) / static_cast<double>(info.totalCount);
    }
    return info;
}

} // namespace sdv
