#ifndef SMARTDATAVISUALISATION_STATISTICSENGINE_H
#define SMARTDATAVISUALISATION_STATISTICSENGINE_H

#include "CategoricalStatistics.h"
#include "CorrelationStatistics.h"
#include "NumericalStatistics.h"

#include "data/DataTable.h"

#include <string>
#include <vector>

namespace sdv {

class StatisticsEngine {
public:
    static NumericalStatisticsResult
    numerical(
        const DataTable& table,
        const std::string& columnName);

    static CategoricalStatisticsResult
    categorical(
        const DataTable& table,
        const std::string& columnName);

    static CorrelationResult
    correlation(
        const DataTable& table,
        const std::string& firstColumnName,
        const std::string& secondColumnName);

    static CorrelationMatrix
    correlationMatrix(
        const DataTable& table,
        const std::vector<std::string>& columnNames);
};

} // namespace sdv

#endif
