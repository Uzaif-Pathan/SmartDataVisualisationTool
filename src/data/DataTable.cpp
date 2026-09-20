#include "DataTable.h"

#include <limits>
#include <stdexcept>

namespace sdv {

DataTable::DataTable()
    : columns_(),
      rows_() {
}

void DataTable::clear() {
    columns_.clear();
    rows_.clear();
}

void DataTable::addColumn(const DataColumn& column) {
    columns_.push_back(column);
}

void DataTable::addRow(const DataRow& row) {
    rows_.push_back(row);
}

const DataColumn& DataTable::columnAt(std::size_t index) const {
    if (index >= columns_.size()) {
        throw std::out_of_range("DataTable column index out of range");
    }

    return columns_[index];
}

DataColumn& DataTable::columnAt(std::size_t index) {
    if (index >= columns_.size()) {
        throw std::out_of_range("DataTable column index out of range");
    }

    return columns_[index];
}

const DataRow& DataTable::rowAt(std::size_t index) const {
    if (index >= rows_.size()) {
        throw std::out_of_range("DataTable row index out of range");
    }

    return rows_[index];
}

DataRow& DataTable::rowAt(std::size_t index) {
    if (index >= rows_.size()) {
        throw std::out_of_range("DataTable row index out of range");
    }

    return rows_[index];
}

const std::vector<DataColumn>& DataTable::columns() const {
    return columns_;
}

const std::vector<DataRow>& DataTable::rows() const {
    return rows_;
}

std::size_t DataTable::columnCount() const {
    return columns_.size();
}

std::size_t DataTable::rowCount() const {
    return rows_.size();
}

bool DataTable::empty() const {
    return columns_.empty() || rows_.empty();
}

std::size_t DataTable::columnIndex(const std::string& name) const {
    for (std::size_t i = 0; i < columns_.size(); ++i) {
        if (columns_[i].name() == name) {
            return i;
        }
    }

    return std::numeric_limits<std::size_t>::max();
}

bool DataTable::hasColumn(const std::string& name) const {
    return columnIndex(name) !=
           std::numeric_limits<std::size_t>::max();
}

} // namespace sdv
