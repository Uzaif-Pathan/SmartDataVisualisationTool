#pragma once
#include <cstddef>
#include <optional>
#include <string>

#include "aggregation/AggregationResult.h"

// Extra calculations built from an existing AggregationResult.
// The input is never modified; a new result is returned.
class DerivedMetrics {
public:
    // Each value as a percentage of the total (for pie/donut charts).
    // Returns nothing if the result is empty, contains a negative value,
    // or the total is zero (percentages would be meaningless).
    static std::optional<AggregationResult> toPercentages(const AggregationResult& input);

    // Running total in the current order.  Only valid for Count/Sum results;
    // throws std::invalid_argument otherwise (a running total of averages
    // has no meaning).
    static AggregationResult cumulative(const AggregationResult& input);

    // Percent change from the previous entry, in the current order (use on
    // time-ordered results).  The first entry has nothing to compare with, so
    // the output is one entry shorter.  Entries whose previous value is 0 are
    // omitted (division by zero).
    static AggregationResult percentChange(const AggregationResult& input);

    // Sorts by value.  Ties keep their original order.
    static AggregationResult sortByValue(const AggregationResult& input,
                                         bool descending = true);

    // Keeps the n largest groups and merges all the rest into one entry
    // called otherLabel.  Only valid for Count/Sum results (merging averages
    // would be wrong); throws std::invalid_argument for other functions or n == 0.
    static AggregationResult topNWithOther(const AggregationResult& input,
                                           size_t n,
                                           const std::string& otherLabel = "Other");
};
