#pragma once

#include "../data/DataColumn.h"

namespace sdv {

struct MissingnessInfo {
    size_t totalCount = 0;
    size_t missingCount = 0;
    double missingPercent = 0.0;
};

// Computes how much of a column is missing/empty.
class MissingnessAnalyzer {
public:
    MissingnessInfo analyze(const DataColumn& column) const;
};

} // namespace sdv
