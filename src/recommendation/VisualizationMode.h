#pragma once

#include <string>

namespace sdv {

// The set of chart types Member 3's C++ graphics module knows how to
// render. This module (recommendation) only ever *chooses* one of
// these — it never draws anything itself.
enum class PlotType {
    None,
    Line,
    Bar,
    GroupedBar,
    StackedBar,
    Scatter,
    Histogram,
    BoxPlot,
    Pie,
    Donut,
    Area,
    Heatmap,
    CorrelationMatrix,
    FrequencyBar
};

std::string plotTypeToString(PlotType type);

} // namespace sdv
