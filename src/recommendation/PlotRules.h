#pragma once

#include <vector>
#include "../profiling/ColumnProfile.h"
#include "VariablePairAnalyzer.h"
#include "VisualizationMode.h"

namespace sdv {

// The actual "what chart makes sense for what data" knowledge base.
// Pure lookup logic — no scoring, no I/O. PlotRecommendationEngine
// consults this, then uses SuitabilityScore to rank the candidates it
// returns.
class PlotRules {
public:
    // Legal chart types for a single column, based on its detected
    // type (e.g. Numeric -> Histogram + BoxPlot; Categorical ->
    // FrequencyBar). Returns an empty list for types that don't
    // support a single-variable chart (Identifier, Unknown).
    std::vector<PlotType> singleColumnPlots(DetectedType type) const;

    // Legal chart types for a pair of columns, based on their
    // relationship as classified by VariablePairAnalyzer. Returns an
    // empty list for InvolvesIdentifier/Unsupported.
    std::vector<PlotType> pairPlots(PairRelationship relationship) const;

    std::string reasonForSingleColumn(DetectedType type, PlotType plot) const;
    std::string reasonForPair(PairRelationship relationship, PlotType plot) const;
};

} // namespace sdv
