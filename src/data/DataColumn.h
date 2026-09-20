#ifndef SMARTDATAVISUALISATION_DATACOLUMN_H
#define SMARTDATAVISUALISATION_DATACOLUMN_H

#include "DataValue.h"
#include "core/Enums.h"

#include <string>
#include <vector>

namespace sdv {

class DataColumn {
public:
    DataColumn();
    explicit DataColumn(const std::string& name);

    const std::string& name() const;
    void setName(const std::string& name);

    DataType dataType() const;
    void setDataType(DataType type);

    void addValue(const DataValue& value);

    const DataValue& valueAt(std::size_t index) const;
    const std::vector<DataValue>& values() const;

    std::size_t size() const;
    bool empty() const;

    std::size_t missingCount() const;
    std::size_t uniqueCount() const;

private:
    std::string name_;
    DataType dataType_;
    std::vector<DataValue> values_;
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_DATACOLUMN_H
