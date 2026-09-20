#pragma once

#include "../data/DataColumn.h"

namespace sdv {

// Decides whether a column is likely an identifier (e.g. Customer_ID,
// row number, transaction reference) rather than a meaningful
// analytical variable.
//
// Primary signal: uniqueness ratio (unique / non-missing values).
// A column where almost every value is unique behaves like a key,
// regardless of its name. The column name is used only as a *minor*
// supporting signal — never the sole basis for the decision, per the
// brief ("column name contains 'id' " is explicitly NOT sufficient).
class IdentifierDetector {
public:
    // Returns a confidence score in [0, 1] that the column is an
    // identifier. >= 0.8 is treated as "likely identifier" by
    // isLikelyIdentifier().
    double identifierScore(const DataColumn& column) const;
    bool isLikelyIdentifier(const DataColumn& column) const;

private:
    static constexpr double kUniquenessWeight = 0.85;
    static constexpr double kNameHintWeight = 0.15;
    static constexpr double kIdentifierThreshold = 0.8;

    // Small bonus if the column name itself suggests an identifier
    // (e.g. contains "id", "code", "number", "key"). Never used alone.
    double nameHintScore(const std::string& columnName) const;
};

} // namespace sdv
