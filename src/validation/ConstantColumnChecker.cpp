#include "ConstantColumnChecker.h"
#include <optional>

namespace sdv {

std::vector<ValidationResult> ConstantColumnChecker::check(const DataTable& table) const {
    std::vector<ValidationResult> results;
    if (table.rowCount() < 2) return results; // need >1 row to call it "constant"

    for (size_t c = 0; c < table.columnCount(); ++c) {
        const DataColumn& col = table.getColumn(c);
        std::optional<std::string> firstValue;
        bool isConstant = true;
        bool sawAny = false;

        for (size_t r = 0; r < col.size(); ++r) {
            const DataValue& v = col.at(r);
            if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
            sawAny = true;
            if (!firstValue) {
                firstValue = v.asString();
            } else if (*firstValue != v.asString()) {
                isConstant = false;
                break;
            }
        }

        if (sawAny && isConstant) {
            results.emplace_back("ConstantColumnChecker", col.name(), Severity::Warning,
                                  "Column has a single constant value (\"" + *firstValue +
                                  "\") across all rows");
        }
    }
    return results;
}

} // namespace sdv
