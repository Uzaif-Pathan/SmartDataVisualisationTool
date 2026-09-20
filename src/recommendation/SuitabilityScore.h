#pragma once

#include "../profiling/ColumnProfile.h"

namespace sdv {

// Small, self-contained scoring functions used by PlotRules to rank
// candidate recommendations. Kept separate from PlotRules so the
// "what combinations are legal" logic (rules) stays readable and
// independent from "how good is this specific combination" (scoring).
class SuitabilityScore {
public:
    // Penalizes columns with a lot of missing data — a chart built on
    // a mostly-empty column is technically legal but not very useful.
    double completenessScore(const ColumnProfile& profile) const;

    // Rewards categorical columns with a manageable number of
    // categories (a bar chart with 3 bars is more readable than one
    // with 40).
    double categoryCountScore(const ColumnProfile& profile) const;

    // Combines the above into a single 0..1 score for a single-column
    // recommendation (Histogram, BoxPlot, Pie/Donut/FrequencyBar).
    double singleColumnScore(const ColumnProfile& profile) const;

    // Combines completeness of both columns into a single 0..1 score
    // for a two-column recommendation (Scatter, Line, GroupedBar...).
    double pairScore(const ColumnProfile& a, const ColumnProfile& b) const;
};

} // namespace sdv
