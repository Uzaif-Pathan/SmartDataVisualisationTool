#include "IdentifierDetector.h"
#include "CardinalityAnalyzer.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace sdv {

double IdentifierDetector::nameHintScore(const std::string& columnName) const {
    std::string lower = columnName;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    static const std::vector<std::string> hints = {"id", "code", "number", "key", "uuid", "no."};
    for (const auto& hint : hints) {
        if (lower.find(hint) != std::string::npos) return 1.0;
    }
    return 0.0;
}

double IdentifierDetector::identifierScore(const DataColumn& column) const {
    CardinalityAnalyzer cardinality;
    CardinalityInfo info = cardinality.analyze(column);

    if (info.nonMissingCount == 0) return 0.0;

    // Uniqueness ratio dominates the decision. A tiny categorical
    // column like Gender (2 uniques / 1000 rows) scores near zero
    // here regardless of its name.
    double uniquenessComponent = info.uniqueRatio;
    double nameComponent = nameHintScore(column.name());

    double score = kUniquenessWeight * uniquenessComponent + kNameHintWeight * nameComponent;
    return std::min(1.0, score);
}

bool IdentifierDetector::isLikelyIdentifier(const DataColumn& column) const {
    return identifierScore(column) >= kIdentifierThreshold;
}

} // namespace sdv
