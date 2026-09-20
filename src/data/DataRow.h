#ifndef SMARTDATAVISUALISATION_DATAROW_H
#define SMARTDATAVISUALISATION_DATAROW_H

#include "DataValue.h"

#include <cstddef>
#include <vector>

namespace sdv {

class DataRow {
public:
    DataRow();

    void addValue(const DataValue& value);

    const DataValue& valueAt(std::size_t index) const;

    const std::vector<DataValue>& values() const;

    std::size_t size() const;
    bool empty() const;

private:
    std::vector<DataValue> values_;
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_DATAROW_H
