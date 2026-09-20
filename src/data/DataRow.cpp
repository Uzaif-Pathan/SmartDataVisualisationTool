#include "DataRow.h"

#include <stdexcept>

namespace sdv {

DataRow::DataRow()
    : values_() {
}

void DataRow::addValue(const DataValue& value) {
    values_.push_back(value);
}

const DataValue& DataRow::valueAt(std::size_t index) const {
    if (index >= values_.size()) {
        throw std::out_of_range("DataRow index out of range");
    }

    return values_[index];
}

const std::vector<DataValue>& DataRow::values() const {
    return values_;
}

std::size_t DataRow::size() const {
    return values_.size();
}

bool DataRow::empty() const {
    return values_.empty();
}

} // namespace sdv
