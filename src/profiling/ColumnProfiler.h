#pragma once

#include <vector>
#include "ColumnProfile.h"
#include "../data/DataTable.h"

namespace sdv {

// Facade for the whole profiling module: runs DataTypeDetector,
// CardinalityAnalyzer, IdentifierDetector, MissingnessAnalyzer,
// VariationAnalyzer and (for categorical columns) CategoricalAnalyzer
// over every column of a DataTable and returns one ColumnProfile per
// column. This is the single entry point the recommendation engine
// (and anything else) should call.
class ColumnProfiler {
public:
    ColumnProfile profileColumn(const DataColumn& column) const;
    std::vector<ColumnProfile> profileTable(const DataTable& table) const;

private:
    static constexpr size_t kTopCategoriesLimit = 10;
};

} // namespace sdv
