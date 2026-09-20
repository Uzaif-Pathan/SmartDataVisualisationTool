#pragma once

#include "ValidationReport.h"
#include "../data/DataTable.h"

namespace sdv {

// Facade that runs every individual checker over a DataTable and
// aggregates the findings into a single ValidationReport. This is the
// only class other modules (e.g. the recommendation engine) should
// need to call directly.
class DataValidator {
public:
    ValidationReport validate(const DataTable& table) const;
};

} // namespace sdv
