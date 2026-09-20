#ifndef SMARTDATAVISUALISATION_STATISTICSTYPES_H
#define SMARTDATAVISUALISATION_STATISTICSTYPES_H

#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace sdv {

struct NumericalStatisticsResult {
    std::size_t count = 0;
    std::size_t missingCount = 0;

    double minimum = 0.0;
    double maximum = 0.0;
    double range = 0.0;

    double mean = 0.0;
    double median = 0.0;
    double mode = 0.0;

    double variance = 0.0;
    double standardDeviation = 0.0;

    double q1 = 0.0;
    double q3 = 0.0;
    double iqr = 0.0;

    std::size_t outlierCount = 0;

    double lowerOutlierFence = 0.0;
    double upperOutlierFence = 0.0;

    double skewness = 0.0;

    bool hasValidStatistics = false;
};

struct CategoryFrequency {
    std::string value;
    std::size_t count = 0;
    double percentage = 0.0;
};

struct CategoricalStatisticsResult {
    std::size_t count = 0;
    std::size_t missingCount = 0;
    std::size_t uniqueCount = 0;

    std::vector<CategoryFrequency> frequencies;

    std::string mostFrequentValue;
    std::size_t mostFrequentCount = 0;

    std::string leastFrequentValue;
    std::size_t leastFrequentCount = 0;

    bool hasValidStatistics = false;
};

struct CorrelationResult {
    std::string firstColumn;
    std::string secondColumn;

    std::size_t pairedCount = 0;

    double covariance = 0.0;
    double pearsonCorrelation = 0.0;

    bool valid = false;
};

struct CorrelationMatrix {
    std::vector<std::string> columnNames;
    std::vector<std::vector<double>> values;
    std::vector<std::vector<bool>> valid;
};

} // namespace sdv

#endif
