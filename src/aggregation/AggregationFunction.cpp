#include "aggregation/AggregationFunction.h"

#include <algorithm>
#include <numeric>

std::string toString(AggregationFunction func) {
    switch (func) {
        case AggregationFunction::Count:  return "Count";
        case AggregationFunction::Sum:    return "Sum";
        case AggregationFunction::Mean:   return "Mean";
        case AggregationFunction::Median: return "Median";
        case AggregationFunction::Min:    return "Min";
        case AggregationFunction::Max:    return "Max";
    }
    return "Unknown";
}

std::optional<double> applyAggregation(AggregationFunction func,
                                       const std::vector<double>& values) {
    if (func == AggregationFunction::Count)
        return static_cast<double>(values.size());
    if (values.empty()) return std::nullopt;

    switch (func) {
        case AggregationFunction::Sum:
            return std::accumulate(values.begin(), values.end(), 0.0);
        case AggregationFunction::Mean:
            return std::accumulate(values.begin(), values.end(), 0.0) /
                   static_cast<double>(values.size());
        case AggregationFunction::Median: {
            std::vector<double> sorted = values;   // copy, so the input is untouched
            std::sort(sorted.begin(), sorted.end());
            const size_t n = sorted.size();
            if (n % 2 == 1) return sorted[n / 2];
            return (sorted[n / 2 - 1] + sorted[n / 2]) / 2.0;
        }
        case AggregationFunction::Min:
            return *std::min_element(values.begin(), values.end());
        case AggregationFunction::Max:
            return *std::max_element(values.begin(), values.end());
        default:
            break;
    }
    return std::nullopt;
}
