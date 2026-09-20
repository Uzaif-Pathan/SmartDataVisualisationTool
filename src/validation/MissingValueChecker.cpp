#include "MissingValueChecker.h"
#include <sstream>
#include <iomanip>

namespace sdv {

std::vector<ValidationResult> MissingValueChecker::check(const DataTable& table) const {
    std::vector<ValidationResult> results;
    const size_t rows = table.rowCount();
    if (rows == 0) return results;

    for (size_t c = 0; c < table.columnCount(); ++c) {
        const DataColumn& col = table.getColumn(c);
        size_t missing = 0;
        for (size_t r = 0; r < col.size(); ++r) {
            if (col.at(r).isMissing() ||
                (col.at(r).isString() && col.at(r).asString().empty())) {
                ++missing;
            }
        }
        if (missing == 0) continue;

        double pct = 100.0 * static_cast<double>(missing) / static_cast<double>(rows);
        std::ostringstream oss;
        oss << "Missing: " << missing << " (" << std::fixed << std::setprecision(1)
            << pct << "%)";

        Severity sev = (missing == rows) ? Severity::Error : Severity::Warning;
        results.emplace_back("MissingValueChecker", col.name(), sev, oss.str());
    }
    return results;
}

} // namespace sdv
