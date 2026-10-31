#include "aggregation/GroupBy.h"

#include <stdexcept>
#include <unordered_map>

GroupByResult groupBy(const DataTable& table, size_t columnIndex) {
    if (columnIndex >= table.columnCount())
        throw std::out_of_range("groupBy: column index out of range");

    GroupByResult result;
    std::unordered_map<std::string, size_t> position;  // label -> index in groups

    for (size_t row = 0; row < table.rowCount(); ++row) {
        if (table.isMissing(row, columnIndex)) {
            ++result.skippedRows;
            continue;
        }
        const std::string label = table.getString(row, columnIndex);

        size_t groupIndex = 0;
        auto it = position.find(label);
        if (it == position.end()) {
            groupIndex = result.groups.size();
            position.emplace(label, groupIndex);
            Group g;
            g.label = label;
            result.groups.push_back(g);
        } else {
            groupIndex = it->second;
        }
        result.groups[groupIndex].rows.push_back(row);
    }
    return result;
}
