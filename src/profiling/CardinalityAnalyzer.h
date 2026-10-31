#pragma once

#include "../data/DataColumn.h"

namespace sdv {

struct CardinalityInfo {
    size_t nonMissingCount = 0;
    size_t uniqueCount = 0;
    double uniqueRatio = 0.0; // uniqueCount / nonMissingCount, 0 if no non-missing values
};

// Counts distinct non-missing values in a column and the ratio of
// unique values to total non-missing values. This ratio is the main
// signal used by IdentifierDetector and CategoricalAnalyzer.
class CardinalityAnalyzer {
public:
    CardinalityInfo analyze(const DataColumn& column) const;
};

} // namespace sdv
