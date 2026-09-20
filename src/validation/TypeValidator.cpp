#include "TypeValidator.h"
#include "ValidationUtils.h"
#include <sstream>
#include <algorithm>

namespace sdv {

std::vector<ValidationResult> TypeValidator::check(const DataTable& table) const {
    std::vector<ValidationResult> results;

    for (size_t c = 0; c < table.columnCount(); ++c) {
        const DataColumn& col = table.getColumn(c);
        size_t nonMissing = 0, numericLike = 0, dateLike = 0, boolLike = 0, textLike = 0;

        for (size_t r = 0; r < col.size(); ++r) {
            const DataValue& v = col.at(r);
            if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
            ++nonMissing;

            const std::string s = v.asString();
            std::string lower = s;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

            if (v.isBoolean() || lower == "true" || lower == "false") {
                ++boolLike;
            } else if (v.isNumeric() || validation_utils::isNumericLiteral(s)) {
                ++numericLike;
            } else if (validation_utils::isDateLiteral(s)) {
                ++dateLike;
            } else {
                ++textLike;
            }
        }

        if (nonMissing == 0) continue;

        size_t dominant = std::max({numericLike, dateLike, boolLike, textLike});
        double ratio = static_cast<double>(dominant) / static_cast<double>(nonMissing);

        if (ratio < kDominantTypeThreshold) {
            std::ostringstream oss;
            oss << "Column has mixed types — numeric: " << numericLike
                << ", date-like: " << dateLike
                << ", boolean: " << boolLike
                << ", text: " << textLike
                << " (out of " << nonMissing << " non-missing values)";
            results.emplace_back("TypeValidator", col.name(), Severity::Warning, oss.str());
        }
    }
    return results;
}

} // namespace sdv
