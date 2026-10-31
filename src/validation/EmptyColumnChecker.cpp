#include "EmptyColumnChecker.h"

namespace sdv {

std::vector<ValidationResult> EmptyColumnChecker::check(const DataTable& table) const {
    std::vector<ValidationResult> results;
    if (table.rowCount() == 0) return results;

    for (size_t c = 0; c < table.columnCount(); ++c) {
        const DataColumn& col = table.getColumn(c);
        bool allEmpty = true;
        for (size_t r = 0; r < col.size(); ++r) {
            const DataValue& v = col.at(r);
            if (!v.isMissing() && !(v.isString() && v.asString().empty())) {
                allEmpty = false;
                break;
            }
        }
        if (allEmpty) {
            results.emplace_back("EmptyColumnChecker", col.name(), Severity::Error,
                                  "Column has no non-empty values across all rows");
        }
    }
    return results;
}

} // namespace sdv
