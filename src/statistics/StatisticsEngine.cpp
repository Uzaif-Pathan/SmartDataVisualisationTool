#include "StatisticsEngine.h"

#include <limits>

namespace sdv {

NumericalStatisticsResult
StatisticsEngine::numerical(
    const DataTable& table,
    const std::string& columnName) {

    const std::size_t index =
        table.columnIndex(columnName);

    if (index >= table.columnCount()) {
        return NumericalStatisticsResult{};
    }

    return NumericalStatistics::calculate(
        table.columnAt(index));
}

CategoricalStatisticsResult
StatisticsEngine::categorical(
    const DataTable& table,
    const std::string& columnName) {

    const std::size_t index =
        table.columnIndex(columnName);

    if (index >= table.columnCount()) {
        return CategoricalStatisticsResult{};
    }

    return CategoricalStatistics::calculate(
        table.columnAt(index));
}

CorrelationResult
StatisticsEngine::correlation(
    const DataTable& table,
    const std::string& firstColumnName,
    const std::string& secondColumnName) {

    const std::size_t firstIndex =
        table.columnIndex(firstColumnName);

    const std::size_t secondIndex =
        table.columnIndex(secondColumnName);

    if (firstIndex >= table.columnCount() ||
        secondIndex >= table.columnCount()) {

        return CorrelationResult{};
    }

    return CorrelationStatistics::calculate(
        table.columnAt(firstIndex),
        table.columnAt(secondIndex));
}

CorrelationMatrix
StatisticsEngine::correlationMatrix(
    const DataTable& table,
    const std::vector<std::string>& columnNames) {

    return CorrelationStatistics::calculateMatrix(
        table,
        columnNames);
}

} // namespace sdv
