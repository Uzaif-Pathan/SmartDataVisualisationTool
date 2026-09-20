#pragma once

#include <vector>
#include "ValidationResult.h"
#include "../data/DataTable.h"

namespace sdv {

// Detects missing values per column and reports counts / percentages.
class MissingValueChecker {
public:
    std::vector<ValidationResult> check(const DataTable& table) const;
};

} // namespace sdv
