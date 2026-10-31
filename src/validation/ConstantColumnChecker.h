#pragma once

#include <vector>
#include "ValidationResult.h"
#include "../data/DataTable.h"

namespace sdv {

// Detects columns where every non-missing value is identical, e.g. a
// "Country" column that is "India" in every row. Such columns carry
// no discriminative information and are later excluded from
// visualization recommendations.
class ConstantColumnChecker {
public:
    std::vector<ValidationResult> check(const DataTable& table) const;
};

} // namespace sdv
