#include "NumericValidator.h"
#include "ValidationUtils.h"
#include <sstream>

namespace sdv {

std::vector<ValidationResult> NumericValidator::check(const DataTable& table) const {
    std::vector<ValidationResult> results;

    for (size_t c = 0; c < table.columnCount(); ++c) {
        const DataColumn& col = table.getColumn(c);
        size_t nonMissing = 0, numeric = 0;

        for (size_t r = 0; r < col.size(); ++r) {
            const DataValue& v = col.at(r);
            if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
            ++nonMissing;
            if (v.isNumeric() || validation_utils::isNumericLiteral(v.asString())) ++numeric;
        }

        if (nonMissing == 0) continue;
        double ratio = static_cast<double>(numeric) / static_cast<double>(nonMissing);
        if (ratio < kNumericMajorityThreshold) continue; // not an "intended numeric" column

        for (size_t r = 0; r < col.size(); ++r) {
            const DataValue& v = col.at(r);
            if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
            bool ok = v.isNumeric() || validation_utils::isNumericLiteral(v.asString());
            if (!ok) {
                std::ostringstream oss;
                oss << "Invalid numeric value \"" << v.asString()
                    << "\" in a predominantly numeric column";
                results.emplace_back("NumericValidator", col.name(), Severity::Error,
                                      oss.str(), static_cast<int>(r));
            }
        }
    }
    return results;
}

} // namespace sdv
