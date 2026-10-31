#pragma once

#include "../data/DataColumn.h"
#include "ColumnProfile.h"

namespace sdv {

// Classifies how much a column's values actually vary, using the
// uniqueness ratio as a fast, type-agnostic proxy (works the same
// way for numeric and categorical columns without needing Member 1's
// variance/stddev calculations).
class VariationAnalyzer {
public:
    VariationLevel classify(const DataColumn& column) const;

private:
    static constexpr double kLowVariationThreshold = 0.05;   // <=5% unique -> low variation
    static constexpr double kHighVariationThreshold = 0.95;  // >=95% unique -> high variation
};

} // namespace sdv
