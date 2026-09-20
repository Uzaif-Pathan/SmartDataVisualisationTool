#pragma once

#include <vector>
#include "ValidationResult.h"
#include "../data/DataTable.h"

namespace sdv {

// Detects columns where every value is missing/empty, e.g. an
// "Unused" column with no data in any row.
class EmptyColumnChecker {
public:
    std::vector<ValidationResult> check(const DataTable& table) const;
};

} // namespace sdv
