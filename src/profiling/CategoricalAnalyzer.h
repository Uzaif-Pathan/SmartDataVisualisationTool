#pragma once

#include <vector>
#include <utility>
#include "../data/DataColumn.h"

namespace sdv {

// Builds a frequency table for a low-cardinality string column, e.g.
// Gender: Male -> 2, Female -> 3. Results are sorted by descending
// frequency.
class CategoricalAnalyzer {
public:
    std::vector<std::pair<std::string, size_t>> frequencyTable(const DataColumn& column) const;

    // Same as frequencyTable() but restricted to the top N entries
    // (useful when a column has many categories).
    std::vector<std::pair<std::string, size_t>> topCategories(const DataColumn& column,
                                                                size_t topN) const;
};

} // namespace sdv
