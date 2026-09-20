#ifndef SMARTDATAVISUALISATION_CORRELATIONSTATISTICS_H
#define SMARTDATAVISUALISATION_CORRELATIONSTATISTICS_H

#include "StatisticsTypes.h"

#include "data/DataColumn.h"
#include "data/DataTable.h"

#include <string>
#include <vector>

namespace sdv {

class CorrelationStatistics {
public:
    static CorrelationResult calculate(
        const DataColumn& firstColumn,
        const DataColumn& secondColumn);

    static CorrelationMatrix calculateMatrix(
        const DataTable& table,
        const std::vector<std::string>& columnNames);
};

} // namespace sdv

#endif
