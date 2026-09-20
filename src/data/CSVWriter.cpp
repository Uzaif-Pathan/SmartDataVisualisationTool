#include "CSVWriter.h"

#include "CSVParser.h"

#include <fstream>
#include <vector>

namespace sdv {

bool CSVWriter::write(
    const std::string& path,
    const DataTable& table,
    std::string& errorMessage) {

    std::ofstream file(path);

    if (!file.is_open()) {
        errorMessage = "Unable to create CSV file.";
        return false;
    }

    std::vector<std::string> headers;

    for (const auto& column : table.columns()) {
        headers.push_back(column.name());
    }

    file << CSVParser::buildLine(headers) << '\n';

    for (const auto& row : table.rows()) {
        std::vector<std::string> values;

        for (const auto& value : row.values()) {
            values.push_back(value.toString());
        }

        file << CSVParser::buildLine(values) << '\n';
    }

    if (!file.good()) {
        errorMessage = "Error occurred while writing CSV.";
        return false;
    }

    return true;
}

} // namespace sdv
