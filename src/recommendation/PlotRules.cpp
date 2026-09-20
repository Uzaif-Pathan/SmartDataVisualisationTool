#include "PlotRules.h"

namespace sdv {

std::vector<PlotType> PlotRules::singleColumnPlots(DetectedType type) const {
    switch (type) {
        case DetectedType::Numeric:
            return {PlotType::Histogram, PlotType::BoxPlot};
        case DetectedType::Categorical:
        case DetectedType::Boolean:
            return {PlotType::FrequencyBar, PlotType::Pie};
        case DetectedType::DateTime:
            // A bare date column with no measure isn't chartable on
            // its own — it needs a paired numeric column (see
            // pairPlots / DateTimeVsNumeric).
            return {};
        case DetectedType::Identifier:
        case DetectedType::Unknown:
        default:
            return {};
    }
}

std::vector<PlotType> PlotRules::pairPlots(PairRelationship relationship) const {
    switch (relationship) {
        case PairRelationship::DateTimeVsNumeric:
            return {PlotType::Line, PlotType::Area};
        case PairRelationship::CategoricalVsNumeric:
            return {PlotType::Bar, PlotType::BoxPlot};
        case PairRelationship::NumericVsNumeric:
            return {PlotType::Scatter};
        case PairRelationship::CategoricalVsCategorical:
            return {PlotType::GroupedBar, PlotType::Heatmap};
        case PairRelationship::InvolvesIdentifier:
        case PairRelationship::Unsupported:
        default:
            return {};
    }
}

std::string PlotRules::reasonForSingleColumn(DetectedType type, PlotType plot) const {
    switch (plot) {
        case PlotType::Histogram:
            return "Numeric variable — histogram shows its distribution";
        case PlotType::BoxPlot:
            return "Numeric variable — box plot summarizes spread and outliers";
        case PlotType::FrequencyBar:
            return "Categorical variable — frequency bar chart shows category counts";
        case PlotType::Pie:
            return "Categorical variable with few categories — pie chart shows proportions";
        default:
            return "Suitable for a " + detectedTypeToString(type) + " variable";
    }
}

std::string PlotRules::reasonForPair(PairRelationship relationship, PlotType plot) const {
    switch (relationship) {
        case PairRelationship::DateTimeVsNumeric:
            return "Date-time variable paired with a numerical measure — " +
                   plotTypeToString(plot) + " shows the trend over time";
        case PairRelationship::CategoricalVsNumeric:
            return "Categorical variable paired with a numerical measure — " +
                   plotTypeToString(plot) + " compares values across categories";
        case PairRelationship::NumericVsNumeric:
            return "Two numerical variables — scatter plot reveals a possible relationship "
                   "(correlation, not causation)";
        case PairRelationship::CategoricalVsCategorical:
            return "Two categorical variables — " + plotTypeToString(plot) +
                   " compares category combinations";
        default:
            return "";
    }
}

} // namespace sdv
