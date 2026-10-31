#include "PlotRecommendationEngine.h"
#include "PlotRules.h"
#include "SuitabilityScore.h"
#include "VariablePairAnalyzer.h"
#include <algorithm>

namespace sdv {

bool PlotRecommendationEngine::isEligibleForAnyChart(const ColumnProfile& profile) {
    // Identifiers, fully-constant columns, and columns we couldn't
    // classify at all are excluded up front — this is what rejects
    // things like Customer_Name/Customer_ID from ever appearing as a
    // recommended axis, per the brief.
    if (profile.type == DetectedType::Identifier) return false;
    if (profile.type == DetectedType::Unknown) return false;
    if (profile.isConstant) return false;
    // A column that's almost entirely missing isn't worth charting.
    if (profile.missingPercent >= 90.0) return false;
    return true;
}

void PlotRecommendationEngine::addSingleColumnRecommendations(
    const RecommendationContext& context, std::vector<RecommendationResult>& out) const {
    PlotRules rules;
    SuitabilityScore scorer;

    for (const auto& profile : context.profiles) {
        if (!isEligibleForAnyChart(profile)) continue;

        auto plots = rules.singleColumnPlots(profile.type);
        double baseScore = scorer.singleColumnScore(profile);

        for (PlotType plot : plots) {
            std::string reason = rules.reasonForSingleColumn(profile.type, plot);
            out.emplace_back(plot, profile.columnName, "", reason, baseScore);
        }
    }
}

void PlotRecommendationEngine::addPairRecommendations(
    const RecommendationContext& context, std::vector<RecommendationResult>& out) const {
    PlotRules rules;
    SuitabilityScore scorer;
    VariablePairAnalyzer pairAnalyzer;

    const auto& profiles = context.profiles;
    for (size_t i = 0; i < profiles.size(); ++i) {
        if (!isEligibleForAnyChart(profiles[i])) continue;

        for (size_t j = i + 1; j < profiles.size(); ++j) {
            if (!isEligibleForAnyChart(profiles[j])) continue;

            PairRelationship relationship = pairAnalyzer.classify(profiles[i], profiles[j]);
            auto plots = rules.pairPlots(relationship);
            if (plots.empty()) continue; // rejected combination (identifier, unsupported, ...)

            double score = scorer.pairScore(profiles[i], profiles[j]);

            // Orient x/y sensibly: date-time or categorical goes on X,
            // numeric goes on Y, for the relationships where that
            // distinction matters.
            const ColumnProfile* xProfile = &profiles[i];
            const ColumnProfile* yProfile = &profiles[j];
            if (relationship == PairRelationship::DateTimeVsNumeric ||
                relationship == PairRelationship::CategoricalVsNumeric) {
                bool iIsMeasure = (profiles[i].type == DetectedType::Numeric);
                if (iIsMeasure) std::swap(xProfile, yProfile);
            }

            for (PlotType plot : plots) {
                std::string reason = rules.reasonForPair(relationship, plot);
                out.emplace_back(plot, xProfile->columnName, yProfile->columnName,
                                  reason, score);
            }
        }
    }
}

std::vector<RecommendationResult> PlotRecommendationEngine::recommend(
    const RecommendationContext& context) const {
    std::vector<RecommendationResult> results;
    addSingleColumnRecommendations(context, results);
    addPairRecommendations(context, results);

    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        return a.suitabilityScore > b.suitabilityScore;
    });
    return results;
}

std::vector<RecommendationResult> PlotRecommendationEngine::recommend(
    const DataTable& table) const {
    return recommend(buildRecommendationContext(table));
}

} // namespace sdv
