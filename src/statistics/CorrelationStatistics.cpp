#include "CorrelationStatistics.h"

#include <cmath>
#include <limits>
#include <utility>

namespace sdv {

CorrelationResult CorrelationStatistics::calculate(
    const DataColumn& firstColumn,
    const DataColumn& secondColumn) {

    CorrelationResult result;

    result.firstColumn = firstColumn.name();
    result.secondColumn = secondColumn.name();

    const std::size_t count =
        std::min(
            firstColumn.size(),
            secondColumn.size());

    std::vector<std::pair<double, double>> pairs;
    pairs.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        const DataValue& first =
            firstColumn.valueAt(i);

        const DataValue& second =
            secondColumn.valueAt(i);

        if (!first.isNumber() ||
            !second.isNumber()) {
            continue;
        }

        const double x = first.asNumber();
        const double y = second.asNumber();

        if (!std::isfinite(x) ||
            !std::isfinite(y)) {
            continue;
        }

        pairs.emplace_back(x, y);
    }

    result.pairedCount = pairs.size();

    if (pairs.size() < 2) {
        return result;
    }

    double sumX = 0.0;
    double sumY = 0.0;

    for (const auto& pair : pairs) {
        sumX += pair.first;
        sumY += pair.second;
    }

    const double n =
        static_cast<double>(pairs.size());

    const double meanX = sumX / n;
    const double meanY = sumY / n;

    double covarianceNumerator = 0.0;
    double sumSquaredX = 0.0;
    double sumSquaredY = 0.0;

    for (const auto& pair : pairs) {
        const double differenceX =
            pair.first - meanX;

        const double differenceY =
            pair.second - meanY;

        covarianceNumerator +=
            differenceX * differenceY;

        sumSquaredX +=
            differenceX * differenceX;

        sumSquaredY +=
            differenceY * differenceY;
    }

    if (pairs.size() > 1) {
        result.covariance =
            covarianceNumerator /
            static_cast<double>(pairs.size() - 1);
    }

    const double denominator =
        std::sqrt(
            sumSquaredX *
            sumSquaredY);

    if (denominator == 0.0) {
        result.pearsonCorrelation = 0.0;
        result.valid = false;
        return result;
    }

    result.pearsonCorrelation =
        covarianceNumerator /
        denominator;

    result.valid = true;

    return result;
}

CorrelationMatrix
CorrelationStatistics::calculateMatrix(
    const DataTable& table,
    const std::vector<std::string>& columnNames) {

    CorrelationMatrix matrix;

    matrix.columnNames = columnNames;

    const std::size_t count =
        columnNames.size();

    matrix.values.assign(
        count,
        std::vector<double>(
            count,
            0.0));

    matrix.valid.assign(
        count,
        std::vector<bool>(
            count,
            false));

    for (std::size_t i = 0; i < count; ++i) {
        const std::size_t firstIndex =
            table.columnIndex(columnNames[i]);

        if (firstIndex >= table.columnCount()) {
            continue;
        }

        matrix.values[i][i] = 1.0;
        matrix.valid[i][i] = true;

        for (std::size_t j = i + 1;
             j < count;
             ++j) {

            const std::size_t secondIndex =
                table.columnIndex(
                    columnNames[j]);

            if (secondIndex >= table.columnCount()) {
                continue;
            }

            const CorrelationResult result =
                calculate(
                    table.columnAt(firstIndex),
                    table.columnAt(secondIndex));

            if (!result.valid) {
                continue;
            }

            matrix.values[i][j] =
                result.pearsonCorrelation;

            matrix.values[j][i] =
                result.pearsonCorrelation;

            matrix.valid[i][j] = true;
            matrix.valid[j][i] = true;
        }
    }

    return matrix;
}

} // namespace sdv

