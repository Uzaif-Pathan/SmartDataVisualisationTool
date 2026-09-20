#pragma once

#include "../profiling/ColumnProfile.h"

namespace sdv {

// The kind of relationship a pair of columns represents, from a
// statistical-visualization point of view. PlotRules maps each of
// these onto one or more legal PlotTypes.
enum class PairRelationship {
    DateTimeVsNumeric,       // time series -> Line/Area
    CategoricalVsNumeric,    // grouped measure -> Bar/GroupedBar/BoxPlot
    NumericVsNumeric,        // continuous relationship -> Scatter
    CategoricalVsCategorical,// cross-tab -> GroupedBar/Heatmap
    InvolvesIdentifier,      // one side is an identifier -> reject
    Unsupported              // no sensible chart (e.g. two free-text columns)
};

// Decides what kind of relationship two columns have, purely from
// their profiles. Does not know about specific chart types.
class VariablePairAnalyzer {
public:
    PairRelationship classify(const ColumnProfile& a, const ColumnProfile& b) const;
};

} // namespace sdv
