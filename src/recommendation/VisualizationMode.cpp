#include "VisualizationMode.h"

namespace sdv {

std::string plotTypeToString(PlotType type) {
    switch (type) {
        case PlotType::None:              return "None";
        case PlotType::Line:              return "LineChart";
        case PlotType::Bar:               return "BarChart";
        case PlotType::GroupedBar:        return "GroupedBarChart";
        case PlotType::StackedBar:        return "StackedBarChart";
        case PlotType::Scatter:           return "ScatterPlot";
        case PlotType::Histogram:         return "Histogram";
        case PlotType::BoxPlot:           return "BoxPlot";
        case PlotType::Pie:               return "PieChart";
        case PlotType::Donut:             return "DonutChart";
        case PlotType::Area:              return "AreaChart";
        case PlotType::Heatmap:           return "Heatmap";
        case PlotType::CorrelationMatrix: return "CorrelationMatrix";
        case PlotType::FrequencyBar:      return "FrequencyBarChart";
    }
    return "None";
}

} // namespace sdv
