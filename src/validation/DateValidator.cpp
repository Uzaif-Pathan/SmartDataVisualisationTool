#include "DateValidator.h"
#include "ValidationUtils.h"
#include <sstream>

namespace sdv {

std::vector<ValidationResult> DateValidator::check(const DataTable& table) const {
    std::vector<ValidationResult> results;

    for (size_t c = 0; c < table.columnCount(); ++c) {
        const DataColumn& col = table.getColumn(c);
        size_t nonMissing = 0, dateLike = 0;

        for (size_t r = 0; r < col.size(); ++r) {
            const DataValue& v = col.at(r);
            if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
            ++nonMissing;
            if (validation_utils::isDateLiteral(v.asString())) ++dateLike;
        }

        if (nonMissing == 0) continue;
        double ratio = static_cast<double>(dateLike) / static_cast<double>(nonMissing);
        if (ratio < kDateMajorityThreshold) continue; // not an "intended date" column

        for (size_t r = 0; r < col.size(); ++r) {
            const DataValue& v = col.at(r);
            if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
            if (!validation_utils::isDateLiteral(v.asString())) {
                std::ostringstream oss;
                oss << "Invalid date value \"" << v.asString()
                    << "\" in a predominantly date column";
                results.emplace_back("DateValidator", col.name(), Severity::Error,
                                      oss.str(), static_cast<int>(r));
            }
        }
    }
    return results;
}

} // namespace sdv
