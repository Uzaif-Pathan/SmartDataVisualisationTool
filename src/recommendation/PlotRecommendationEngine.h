#pragma once

#include <vector>
#include "RecommendationContext.h"
#include "RecommendationResult.h"

namespace sdv {

// The public entry point for Member 2's recommendation module.
// Given a dataset, answers: "which visualizations actually make
// statistical sense for this data?" It never draws anything — it
// only returns PlotType + column names + a reason + a score. Member
// 3's C++ graphics module consumes RecommendationResult and renders
// it.
class PlotRecommendationEngine {
public:
    // Build recommendations directly from a table (profiles the table
    // internally).
    std::vector<RecommendationResult> recommend(const DataTable& table) const;

    // Build recommendations from an already-built context (e.g. if
    // the caller already has profiles from elsewhere and wants to
    // avoid re-profiling).
    std::vector<RecommendationResult> recommend(const RecommendationContext& context) const;

private:
    void addSingleColumnRecommendations(const RecommendationContext& context,
                                         std::vector<RecommendationResult>& out) const;
    void addPairRecommendations(const RecommendationContext& context,
                                 std::vector<RecommendationResult>& out) const;

    static bool isEligibleForAnyChart(const ColumnProfile& profile);
};

} // namespace sdv
