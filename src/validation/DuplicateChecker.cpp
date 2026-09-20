#include "DuplicateChecker.h"
#include <unordered_map>
#include <sstream>

namespace sdv {

namespace {
// Build a simple concatenated key for a row. Good enough for exact-
// duplicate detection without pulling in a hashing library.
std::string rowKey(const DataTable& table, size_t rowIdx) {
    std::ostringstream oss;
    for (size_t c = 0; c < table.columnCount(); ++c) {
        oss << table.getValue(rowIdx, c).asString() << '\x1f'; // unit separator
    }
    return oss.str();
}
} // namespace

std::vector<ValidationResult> DuplicateChecker::check(const DataTable& table) const {
    std::vector<ValidationResult> results;
    std::unordered_map<std::string, int> firstSeenRow; // key -> first row index

    for (size_t r = 0; r < table.rowCount(); ++r) {
        std::string key = rowKey(table, r);
        auto it = firstSeenRow.find(key);
        if (it == firstSeenRow.end()) {
            firstSeenRow[key] = static_cast<int>(r);
        } else {
            std::ostringstream oss;
            oss << "Row " << r << " is a duplicate of row " << it->second;
            results.emplace_back("DuplicateChecker", "", Severity::Warning,
                                  oss.str(), static_cast<int>(r));
        }
    }
    return results;
}

} // namespace sdv
