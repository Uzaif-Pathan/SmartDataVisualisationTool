#ifndef SMARTDATAVISUALISATION_CONSTANTS_H
#define SMARTDATAVISUALISATION_CONSTANTS_H

namespace sdv::constants {

// Application
constexpr const char* APPLICATION_NAME =
    "Smart Data Visualisation Tool";

constexpr const char* APPLICATION_VERSION =
    "1.0.0";

// Supported data
constexpr std::size_t DEFAULT_SAMPLE_SIZE = 10000;
constexpr std::size_t MAX_DISPLAY_CATEGORIES = 20;
constexpr std::size_t MAX_SCATTER_POINTS = 10000;

// Statistical thresholds
constexpr double IDENTIFIER_UNIQUENESS_THRESHOLD = 0.95;
constexpr double OUTLIER_IQR_MULTIPLIER = 1.5;
constexpr double HIGH_MISSINGNESS_THRESHOLD = 0.50;

// Visualization
constexpr std::size_t DEFAULT_HISTOGRAM_BINS = 10;
constexpr std::size_t MAX_RECOMMENDED_VISUALIZATIONS = 12;

// Storage
constexpr const char* PROJECTS_DIRECTORY = "output/projects";
constexpr const char* PLOTS_DIRECTORY = "plots";
constexpr const char* ANALYSIS_DIRECTORY = "analysis";
constexpr const char* METADATA_DIRECTORY = "metadata";

} // namespace sdv::constants

#endif // SMARTDATAVISUALISATION_CONSTANTS_H
