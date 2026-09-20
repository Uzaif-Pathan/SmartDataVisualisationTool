#pragma once

#include <vector>
#include "ValidationResult.h"
#include "../data/DataTable.h"

namespace sdv {

// Flags individual values that break an otherwise-numeric column,
// e.g. "abc" inside a Sales column of numbers.
//
// Heuristic: a column is considered "intended numeric" if at least
// 60% of its non-missing values parse as numbers. Any remaining
// non-missing, non-numeric value in such a column is reported.
class NumericValidator {
public:
    std::vector<ValidationResult> check(const DataTable& table) const;

private:
    static constexpr double kNumericMajorityThreshold = 0.6;
};

} // namespace sdv
