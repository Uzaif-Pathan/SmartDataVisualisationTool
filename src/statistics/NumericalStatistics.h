#ifndef SMARTDATAVISUALISATION_NUMERICALSTATISTICS_H
#define SMARTDATAVISUALISATION_NUMERICALSTATISTICS_H

#include "StatisticsTypes.h"

#include "data/DataColumn.h"

#include <vector>

namespace sdv {

class NumericalStatistics {
public:
    static NumericalStatisticsResult calculate(
        const DataColumn& column);

    static NumericalStatisticsResult calculate(
        const std::vector<double>& values);

private:
    static double percentile(
        std::vector<double> values,
        double percentileValue);

    static double calculateSkewness(
        const std::vector<double>& values,
        double mean,
        double standardDeviation);
};

} // namespace sdv

#endif
