#pragma once
#include <optional>
#include <string>
#include <vector>

// The six operations the project supports.
enum class AggregationFunction { Count, Sum, Mean, Median, Min, Max };

// Human-readable name, e.g. AggregationFunction::Sum -> "Sum".
std::string toString(AggregationFunction func);

// Applies one operation to a list of numbers.
//  - Count returns how many numbers there are (0 for an empty list).
//  - Every other operation returns nothing (std::nullopt) for an empty list.
std::optional<double> applyAggregation(AggregationFunction func,
                                       const std::vector<double>& values);
