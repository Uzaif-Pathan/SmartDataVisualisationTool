#pragma once

#include <vector>
#include "ValidationResult.h"
#include "../data/DataTable.h"

namespace sdv {

// Flags individual values that break an otherwise-date column,
// e.g. "not-a-date" inside a column of ISO dates.
//
// Heuristic: a column is considered "intended date" if at least 60%
// of its non-missing values match a supported date format.
class DateValidator {
public:
    std::vector<ValidationResult> check(const DataTable& table) const;

private:
    static constexpr double kDateMajorityThreshold = 0.6;
};

} // namespace sdv
