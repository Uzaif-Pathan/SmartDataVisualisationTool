#pragma once

#include <string>
#include "VisualizationMode.h"

namespace sdv {

// One suggested visualization, handed off to Member 3's graphics
// module. yColumn is empty for single-variable plots (Histogram,
// BoxPlot, Pie/Donut/FrequencyBar over one categorical column).
struct RecommendationResult {
    PlotType plotType = PlotType::None;
    std::string xColumn;
    std::string yColumn;      // empty if not applicable
    std::string reason;       // human-readable justification
    double suitabilityScore = 0.0; // 0..1, higher = more strongly recommended

    RecommendationResult() = default;
    RecommendationResult(PlotType type, std::string x, std::string y,
                          std::string why, double score)
        : plotType(type), xColumn(std::move(x)), yColumn(std::move(y)),
          reason(std::move(why)), suitabilityScore(score) {}

    std::string describe() const;
};

} // namespace sdv
