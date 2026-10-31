#pragma once

#include <vector>
#include "ValidationResult.h"
#include "../data/DataTable.h"

namespace sdv {

// Flags columns whose values do not settle on a single dominant type
// (e.g. a mix of numbers and free text with no clear majority). This
// is a coarser, column-level check than NumericValidator/DateValidator,
// which operate value-by-value once a dominant type is established.
class TypeValidator {
public:
    std::vector<ValidationResult> check(const DataTable& table) const;

private:
    // If the largest type group holds less than this fraction of
    // non-missing values, the column is reported as "mixed type".
    static constexpr double kDominantTypeThreshold = 0.6;
};

} // namespace sdv
