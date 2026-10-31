#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "data/DataTable.h"

// One group: a label plus the row numbers that belong to it.
struct Group {
    std::string label;
    std::vector<size_t> rows;
};

struct GroupByResult {
    std::vector<Group> groups;   // in order of first appearance
    size_t skippedRows = 0;      // rows whose group value was missing
};

// Groups all rows of the table by the value in one column.
// Throws std::out_of_range if columnIndex is not a valid column.
GroupByResult groupBy(const DataTable& table, size_t columnIndex);
