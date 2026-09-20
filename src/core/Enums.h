#ifndef SMARTDATAVISUALISATION_ENUMS_H
#define SMARTDATAVISUALISATION_ENUMS_H

namespace sdv {

enum class DataType {
    Unknown,
    Numeric,
    Categorical,
    DateTime,
    Boolean,
    Identifier
};

enum class VisualizationMode {
    Manual,
    Assisted,
    Automatic
};

enum class PlotType {
    Automatic,
    Bar,
    GroupedBar,
    StackedBar,
    Line,
    Scatter,
    Histogram,
    BoxPlot,
    Pie,
    Donut,
    Area,
    Heatmap,
    CorrelationMatrix
};

enum class AggregationType {
    Count,
    Sum,
    Mean,
    Median,
    Minimum,
    Maximum
};

enum class ApplicationPage {
    Home,
    NewVisualization,
    ResultViewer,
    Settings
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_ENUMS_H
