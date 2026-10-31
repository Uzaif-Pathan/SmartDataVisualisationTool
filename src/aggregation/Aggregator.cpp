#include "aggregation/Aggregator.h"

#include <cmath>
#include <stdexcept>
#include <vector>

#include "aggregation/GroupBy.h"

namespace {
size_t requireColumn(const DataTable& table, const std::string& name) {
    const int index = table.columnIndex(name);
    if (index < 0) throw std::invalid_argument("Column not found: " + name);
    return static_cast<size_t>(index);
}
}  // namespace

AggregationResult Aggregator::countByGroup(const DataTable& table,
                                           const std::string& groupColumn) {
    const size_t groupIdx = requireColumn(table, groupColumn);
    const GroupByResult grouped = groupBy(table, groupIdx);

    AggregationResult result;
    result.groupColumn = groupColumn;
    result.function = AggregationFunction::Count;
    result.skippedRows = grouped.skippedRows;
    for (const Group& g : grouped.groups) {
        result.labels.push_back(g.label);
        result.values.push_back(static_cast<double>(g.rows.size()));
    }
    return result;
}

AggregationResult Aggregator::aggregate(const DataTable& table,
                                        const std::string& groupColumn,
                                        const std::string& valueColumn,
                                        AggregationFunction func) {
    const size_t groupIdx = requireColumn(table, groupColumn);
    const size_t valueIdx = requireColumn(table, valueColumn);
    const GroupByResult grouped = groupBy(table, groupIdx);

    AggregationResult result;
    result.groupColumn = groupColumn;
    result.valueColumn = valueColumn;
    result.function = func;
    result.skippedRows = grouped.skippedRows;

    for (const Group& g : grouped.groups) {
        std::vector<double> numbers;
        for (size_t row : g.rows) {
            const auto v = table.getNumber(row, valueIdx);
            if (v && std::isfinite(*v)) numbers.push_back(*v);
            else ++result.skippedRows;
        }
        const auto value = applyAggregation(func, numbers);
        if (!value) continue;   // group had no usable numbers
        result.labels.push_back(g.label);
        result.values.push_back(*value);
    }
    return result;
}
