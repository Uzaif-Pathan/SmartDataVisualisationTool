#pragma once
#include <string>

#include "aggregation/AggregationFunction.h"
#include "aggregation/AggregationResult.h"
#include "data/DataTable.h"

class Aggregator {
public:
    // Counts rows per category.  Gender -> Male 5, Female 5.
    // Throws std::invalid_argument if the column name does not exist.
    static AggregationResult countByGroup(const DataTable& table,
                                          const std::string& groupColumn);

    // Applies func to valueColumn separately for each group.
    // Empty or non-numeric cells are skipped and counted in skippedRows.
    // Throws std::invalid_argument if a column name does not exist.
    static AggregationResult aggregate(const DataTable& table,
                                       const std::string& groupColumn,
                                       const std::string& valueColumn,
                                       AggregationFunction func);
};
