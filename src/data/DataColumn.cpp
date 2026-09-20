#include "DataColumn.h"

#include <set>
#include <stdexcept>

namespace sdv {

DataColumn::DataColumn()
    : name_(),
      dataType_(DataType::Unknown),
      values_() {
}

DataColumn::DataColumn(const std::string& name)
    : name_(name),
      dataType_(DataType::Unknown),
      values_() {
}

const std::string& DataColumn::name() const {
    return name_;
}

void DataColumn::setName(const std::string& name) {
    name_ = name;
}

DataType DataColumn::dataType() const {
    return dataType_;
}

void DataColumn::setDataType(DataType type) {
    dataType_ = type;
}

void DataColumn::addValue(const DataValue& value) {
    values_.push_back(value);
}

const DataValue& DataColumn::valueAt(std::size_t index) const {
    if (index >= values_.size()) {
        throw std::out_of_range("DataColumn index out of range");
    }

    return values_[index];
}

const std::vector<DataValue>& DataColumn::values() const {
    return values_;
}

std::size_t DataColumn::size() const {
    return values_.size();
}

bool DataColumn::empty() const {
    return values_.empty();
}

std::size_t DataColumn::missingCount() const {
    std::size_t count = 0;

    for (const auto& value : values_) {
        if (value.isEmpty()) {
            ++count;
        }
    }

    return count;
}

std::size_t DataColumn::uniqueCount() const {
    std::set<std::string> uniqueValues;

    for (const auto& value : values_) {
        if (!value.isEmpty()) {
            uniqueValues.insert(value.toString());
        }
    }

    return uniqueValues.size();
}

} // namespace sdv
