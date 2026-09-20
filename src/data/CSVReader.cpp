#include "CSVReader.h"

#include "CSVParser.h"
#include "utils/NumberUtils.h"
#include "utils/StringUtils.h"

#include <fstream>

namespace sdv {

bool CSVReader::read(
    const std::string& path,
    DataTable& table,
    std::string& errorMessage) {

    std::ifstream file(path);

    if (!file.is_open()) {
        errorMessage = "Unable to open CSV file.";
        return false;
    }

    table.clear();

    std::string line;

    if (!std::getline(file, line)) {
        errorMessage = "CSV file is empty.";
        return false;
    }

    const auto headers = CSVParser::parseLine(line);

    if (headers.empty()) {
        errorMessage = "CSV contains no columns.";
        return false;
    }

    for (const auto& header : headers) {
        DataColumn column(
            utils::trim(header));

        table.addColumn(column);
    }

    while (std::getline(file, line)) {
        const auto fields =
            CSVParser::parseLine(line);

        DataRow row;

        for (std::size_t i = 0;
             i < headers.size();
             ++i) {

            if (i < fields.size()) {
                row.addValue(
                    convertValue(
                        utils::trim(fields[i])));
            }
            else {
                row.addValue(DataValue());
            }
        }

        table.addRow(row);
    }

    // Populate column values from rows.
    for (std::size_t rowIndex = 0;
         rowIndex < table.rowCount();
         ++rowIndex) {

        const DataRow& row =
            table.rowAt(rowIndex);

        for (std::size_t columnIndex = 0;
             columnIndex < table.columnCount();
             ++columnIndex) {

            table.columnAt(columnIndex)
                .addValue(
                    row.valueAt(columnIndex));
        }
    }

    return true;
}

DataValue CSVReader::convertValue(
    const std::string& value) {

    if (value.empty()) {
        return DataValue();
    }

    const std::string lower =
        utils::toLower(value);

    if (lower == "true") {
        return DataValue(true);
    }

    if (lower == "false") {
        return DataValue(false);
    }

    if (utils::isNumber(value)) {
        bool success = false;

        const double number =
            utils::toDouble(value, &success);

        if (success) {
            return DataValue(number);
        }
    }

    return DataValue(value);
}

} // namespace sdv
