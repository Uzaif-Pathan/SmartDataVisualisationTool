#ifndef SMARTDATAVISUALISATION_CATEGORICALSTATISTICS_H
#define SMARTDATAVISUALISATION_CATEGORICALSTATISTICS_H

#include "StatisticsTypes.h"

#include "data/DataColumn.h"

namespace sdv {

class CategoricalStatistics {
public:
    static CategoricalStatisticsResult calculate(
        const DataColumn& column);
};

} // namespace sdv

#endif
