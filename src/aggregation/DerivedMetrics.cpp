#include "aggregation/DerivedMetrics.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace {

bool isAdditive(AggregationFunction f) {
    return f == AggregationFunction::Count || f == AggregationFunction::Sum;
}

// Same column names/function as the input, but no data yet.
AggregationResult emptyLike(const AggregationResult& in) {
    AggregationResult out;
    out.groupColumn = in.groupColumn;
    out.valueColumn = in.valueColumn;
    out.function = in.function;
    out.skippedRows = in.skippedRows;
    return out;
}

// e.g. "Sales" + " (% of total)"; plain row counts are called "Rows".
std::string describe(const std::string& valueColumn, const std::string& suffix) {
    return (valueColumn.empty() ? std::string("Rows") : valueColumn) + suffix;
}

}  // namespace

std::optional<AggregationResult> DerivedMetrics::toPercentages(const AggregationResult& input) {
    if (input.empty()) return std::nullopt;
    double total = 0.0;
    for (double v : input.values) {
        if (v < 0.0) return std::nullopt;
        total += v;
    }
    if (total <= 0.0) return std::nullopt;

    AggregationResult out = emptyLike(input);
    out.valueColumn = describe(input.valueColumn, " (% of total)");
    out.labels = input.labels;
    for (double v : input.values) out.values.push_back(v / total * 100.0);
    return out;
}

AggregationResult DerivedMetrics::cumulative(const AggregationResult& input) {
    if (!isAdditive(input.function))
        throw std::invalid_argument("cumulative() only works for Count or Sum results");

    AggregationResult out = emptyLike(input);
    out.valueColumn = describe(input.valueColumn, " (cumulative)");
    out.labels = input.labels;
    double running = 0.0;
    for (double v : input.values) {
        running += v;
        out.values.push_back(running);
    }
    return out;
}

AggregationResult DerivedMetrics::percentChange(const AggregationResult& input) {
    AggregationResult out = emptyLike(input);
    out.valueColumn = describe(input.valueColumn, " (% change)");
    for (size_t i = 1; i < input.values.size(); ++i) {
        const double previous = input.values[i - 1];
        if (previous == 0.0) continue;
        out.labels.push_back(input.labels[i]);
        out.values.push_back((input.values[i] - previous) / std::fabs(previous) * 100.0);
    }
    return out;
}

AggregationResult DerivedMetrics::sortByValue(const AggregationResult& input, bool descending) {
    std::vector<size_t> order(input.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(), [&](size_t a, size_t b) {
        return descending ? input.values[a] > input.values[b]
                          : input.values[a] < input.values[b];
    });

    AggregationResult out = emptyLike(input);
    for (size_t i : order) {
        out.labels.push_back(input.labels[i]);
        out.values.push_back(input.values[i]);
    }
    return out;
}

AggregationResult DerivedMetrics::topNWithOther(const AggregationResult& input,
                                                size_t n,
                                                const std::string& otherLabel) {
    if (n == 0) throw std::invalid_argument("topNWithOther(): n must be at least 1");
    if (!isAdditive(input.function))
        throw std::invalid_argument("topNWithOther() only works for Count or Sum results");

    const AggregationResult sorted = sortByValue(input, true);
    if (sorted.size() <= n) return sorted;

    AggregationResult out = emptyLike(input);
    for (size_t i = 0; i < n; ++i) {
        out.labels.push_back(sorted.labels[i]);
        out.values.push_back(sorted.values[i]);
    }
    double other = 0.0;
    for (size_t i = n; i < sorted.size(); ++i) other += sorted.values[i];
    out.labels.push_back(otherLabel);
    out.values.push_back(other);
    return out;
}
