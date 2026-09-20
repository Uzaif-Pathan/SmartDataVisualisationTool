#include "NumericalStatistics.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>

namespace sdv {

NumericalStatisticsResult NumericalStatistics::calculate(
    const DataColumn& column) {

    std::vector<double> values;
    values.reserve(column.size());

    std::size_t missingCount = 0;

    for (const DataValue& value : column.values()) {
        if (value.isNumber()) {
            values.push_back(value.asNumber());
        } else {
            ++missingCount;
        }
    }

    NumericalStatisticsResult result =
        calculate(values);

    result.missingCount = missingCount;

    return result;
}

NumericalStatisticsResult NumericalStatistics::calculate(
    const std::vector<double>& values) {

    NumericalStatisticsResult result;

    result.count = values.size();

    if (values.empty()) {
        return result;
    }

    result.minimum =
        *std::min_element(values.begin(), values.end());

    result.maximum =
        *std::max_element(values.begin(), values.end());

    result.range =
        result.maximum - result.minimum;

    double sum = 0.0;

    for (double value : values) {
        sum += value;
    }

    result.mean =
        sum / static_cast<double>(values.size());

    result.median =
        percentile(values, 50.0);

    result.q1 =
        percentile(values, 25.0);

    result.q3 =
        percentile(values, 75.0);

    result.iqr =
        result.q3 - result.q1;

    result.lowerOutlierFence =
        result.q1 - (1.5 * result.iqr);

    result.upperOutlierFence =
        result.q3 + (1.5 * result.iqr);

    std::size_t outlierCount = 0;

    for (double value : values) {
        if (value < result.lowerOutlierFence ||
            value > result.upperOutlierFence) {
            ++outlierCount;
        }
    }

    result.outlierCount = outlierCount;

    std::map<double, std::size_t> frequency;

    for (double value : values) {
        ++frequency[value];
    }

    std::size_t highestFrequency = 0;
    double modeValue = values.front();

    for (const auto& entry : frequency) {
        if (entry.second > highestFrequency) {
            highestFrequency = entry.second;
            modeValue = entry.first;
        }
    }

    result.mode = modeValue;

    if (values.size() > 1) {
        double squaredDifferenceSum = 0.0;

        for (double value : values) {
            const double difference =
                value - result.mean;

            squaredDifferenceSum +=
                difference * difference;
        }

        result.variance =
            squaredDifferenceSum /
            static_cast<double>(values.size() - 1);

        result.standardDeviation =
            std::sqrt(result.variance);
    } else {
        result.variance = 0.0;
        result.standardDeviation = 0.0;
    }

    result.skewness =
        calculateSkewness(
            values,
            result.mean,
            result.standardDeviation);

    result.hasValidStatistics = true;

    return result;
}

double NumericalStatistics::percentile(
    std::vector<double> values,
    double percentileValue) {

    if (values.empty()) {
        return 0.0;
    }

    std::sort(values.begin(), values.end());

    if (values.size() == 1) {
        return values.front();
    }

    const double position =
        (percentileValue / 100.0) *
        static_cast<double>(values.size() - 1);

    const std::size_t lowerIndex =
        static_cast<std::size_t>(
            std::floor(position));

    const std::size_t upperIndex =
        static_cast<std::size_t>(
            std::ceil(position));

    if (lowerIndex == upperIndex) {
        return values[lowerIndex];
    }

    const double fraction =
        position - static_cast<double>(lowerIndex);

    return values[lowerIndex] +
           fraction *
               (values[upperIndex] -
                values[lowerIndex]);
}

double NumericalStatistics::calculateSkewness(
    const std::vector<double>& values,
    double mean,
    double standardDeviation) {

    if (values.size() < 3 ||
        standardDeviation == 0.0) {
        return 0.0;
    }

    double sumCubed = 0.0;

    for (double value : values) {
        const double standardized =
            (value - mean) / standardDeviation;

        sumCubed +=
            standardized *
            standardized *
            standardized;
    }

    const double n =
        static_cast<double>(values.size());

    return
        (n / ((n - 1.0) * (n - 2.0))) *
        sumCubed;
}

} // namespace sdv
