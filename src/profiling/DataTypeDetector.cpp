#include "DataTypeDetector.h"
#include "IdentifierDetector.h"
#include "DateTimeDetector.h"
#include "NumericAnalyzer.h"
#include "CardinalityAnalyzer.h"
#include <algorithm>
#include <string>
#include <vector>

namespace sdv {

bool DataTypeDetector::isLikelyBoolean(const DataColumn& column) const {
    size_t nonMissing = 0, boolLike = 0;
    for (size_t i = 0; i < column.size(); ++i) {
        const DataValue& v = column.at(i);
        if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
        ++nonMissing;

        if (v.isBoolean()) {
            ++boolLike;
            continue;
        }
        std::string lower = v.asString();
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (lower == "true" || lower == "false" || lower == "yes" || lower == "no") {
            ++boolLike;
        }
    }
    if (nonMissing == 0) return false;
    return static_cast<double>(boolLike) / static_cast<double>(nonMissing) >= 0.9;
}

namespace {
// Very high uniqueness on its own is NOT sufficient evidence of an
// identifier: a small/clean Sales column or a column of daily Dates
// can easily be 100% unique without being a "key". A name hint is
// used only as a *secondary* confirming signal here, exactly as the
// brief allows ("supporting evidence, but not the only rule") — it is
// combined with, never substituted for, the uniqueness signal.
bool nameSuggestsIdentifier(const std::string& columnName) {
    std::string lower = columnName;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    static const std::vector<std::string> hints = {"id", "code", "key", "uuid", "no."};
    for (const auto& hint : hints) {
        if (lower.find(hint) != std::string::npos) return true;
    }
    return false;
}
} // namespace

DetectedType DataTypeDetector::detect(const DataColumn& column) const {
    CardinalityAnalyzer cardinality;
    CardinalityInfo cardInfo = cardinality.analyze(column);
    if (cardInfo.nonMissingCount == 0) return DetectedType::Unknown;

    // Dates and numeric measures are checked BEFORE identifier status:
    // a column of well-formed dates or well-formed numbers is never
    // an identifier in this domain, no matter how unique its values
    // are (a daily Date column, or a clean Sales column with no
    // repeats, are both 100% unique by nature — that uniqueness
    // reflects the data being *continuous*, not the column being a
    // *key*).
    DateTimeDetector dateDetector;
    if (dateDetector.isLikelyDateTime(column)) return DetectedType::DateTime;

    NumericAnalyzer numericAnalyzer;
    if (numericAnalyzer.isLikelyNumeric(column)) {
        // Rare exception: a numeric column that is both near-fully
        // unique AND whose name strongly hints at being a key (e.g.
        // "Customer_ID" happens to be stored as plain numbers) is
        // still recognized as an identifier. Both signals must agree.
        if (cardInfo.uniqueRatio >= 0.9 && nameSuggestsIdentifier(column.name())) {
            return DetectedType::Identifier;
        }
        return DetectedType::Numeric;
    }

    if (isLikelyBoolean(column)) return DetectedType::Boolean;

    // Everything left is non-numeric, non-date text. NOW uniqueness
    // is a meaningful identifier signal (e.g. Customer_Name,
    // Customer_ID stored as strings, transaction references).
    IdentifierDetector identifierDetector;
    if (identifierDetector.isLikelyIdentifier(column)) return DetectedType::Identifier;

    // Low/moderate cardinality text -> categorical. High-cardinality
    // free text that still isn't an identifier (e.g. long comments)
    // falls through to Unknown.
    if (cardInfo.uniqueRatio <= 0.5) return DetectedType::Categorical;

    return DetectedType::Unknown;
}

} // namespace sdv
