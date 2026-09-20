#pragma once

#include "../data/DataColumn.h"

namespace sdv {

// NOTE: this is a *profiling*-level analyzer — it only asks "does
// this column look numeric?" for type-detection purposes. Actual
// numeric statistics (mean, stddev, quantiles, etc.) belong to
// Member 1's src/statistics/ module.
class NumericAnalyzer {
public:
    // Fraction (0..1) of non-missing values that parse as numbers.
    double numericLikeRatio(const DataColumn& column) const;
    bool isLikelyNumeric(const DataColumn& column, double threshold = 0.6) const;
};

} // namespace sdv
