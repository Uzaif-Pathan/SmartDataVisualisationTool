#pragma once

#include <vector>
#include "ValidationResult.h"
#include "../data/DataTable.h"

namespace sdv {

// Detects rows that are exact duplicates of an earlier row (all
// column values equal).
class DuplicateChecker {
public:
    std::vector<ValidationResult> check(const DataTable& table) const;
};

} // namespace sdv
