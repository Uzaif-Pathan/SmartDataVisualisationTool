#pragma once
#include <cstddef>
#include <string>
#include <vector>

#include "aggregation/AggregationFunction.h"

// The output of an aggregation: one label and one value per group.
// Example: labels = {"Male", "Female"}, values = {5, 5}.
struct AggregationResult {
    std::string groupColumn;     // e.g. "Gender"
    std::string valueColumn;     // e.g. "Sales" (empty for plain row counts)
    AggregationFunction function = AggregationFunction::Count;

    std::vector<std::string> labels;   // one per group
    std::vector<double> values;        // same length as labels

    // Rows that could not be used (missing group or unusable number).
    // Reported instead of silently thrown away.
    size_t skippedRows = 0;

    size_t size() const { return labels.size(); }
    bool empty() const { return labels.empty(); }

    // Readable summary, handy for testing and for the "why this graph" text.
    std::string toText() const;
};
