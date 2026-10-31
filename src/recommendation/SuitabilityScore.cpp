#include "SuitabilityScore.h"
#include <algorithm>

namespace sdv {

double SuitabilityScore::completenessScore(const ColumnProfile& profile) const {
    // 0% missing -> 1.0, 100% missing -> 0.0, linear in between.
    double score = 1.0 - (profile.missingPercent / 100.0);
    return std::clamp(score, 0.0, 1.0);
}

double SuitabilityScore::categoryCountScore(const ColumnProfile& profile) const {
    if (profile.type != DetectedType::Categorical) return 1.0; // n/a, don't penalize
    size_t n = profile.uniqueCount;
    if (n == 0) return 0.0;
    if (n <= 8) return 1.0;         // ideal bar/pie chart size
    if (n <= 20) return 0.7;        // still readable, a bit busy
    return 0.4;                     // very busy chart, still legal but discouraged
}

double SuitabilityScore::singleColumnScore(const ColumnProfile& profile) const {
    double score = 0.6 * completenessScore(profile) + 0.4 * categoryCountScore(profile);
    return std::clamp(score, 0.0, 1.0);
}

double SuitabilityScore::pairScore(const ColumnProfile& a, const ColumnProfile& b) const {
    double completeness = 0.5 * (completenessScore(a) + completenessScore(b));
    double categoryPenalty = 0.5 * (categoryCountScore(a) + categoryCountScore(b));
    double score = 0.6 * completeness + 0.4 * categoryPenalty;
    return std::clamp(score, 0.0, 1.0);
}

} // namespace sdv
