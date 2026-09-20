#ifndef SMARTDATAVISUALISATION_DATATABLE_H
#define SMARTDATAVISUALISATION_DATATABLE_H

#include "DataColumn.h"
#include "DataRow.h"

#include <cstddef>
#include <string>
#include <vector>

namespace sdv {

class DataTable {
public:
    DataTable();

    void clear();

    void addColumn(const DataColumn& column);
    void addRow(const DataRow& row);

    const DataColumn& columnAt(std::size_t index) const;
    DataColumn& columnAt(std::size_t index);

    const DataRow& rowAt(std::size_t index) const;
    DataRow& rowAt(std::size_t index);

    const std::vector<DataColumn>& columns() const;
    const std::vector<DataRow>& rows() const;

    std::size_t columnCount() const;
    std::size_t rowCount() const;

    bool empty() const;

    std::size_t columnIndex(const std::string& name) const;

    bool hasColumn(const std::string& name) const;

private:
    std::vector<DataColumn> columns_;
    std::vector<DataRow> rows_;
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_DATATABLE_H
