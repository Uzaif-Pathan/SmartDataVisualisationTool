#include "DataLoader.h"

#include "CSVReader.h"
#include "ExcelReader.h"
#include "FileFormatDetector.h"

namespace sdv {

bool DataLoader::load(
    const std::string& path,
    DataTable& table,
    std::string& errorMessage) {

    table.clear();
    errorMessage.clear();

    const DataSourceType type =
        FileFormatDetector::detect(path);

    switch (type) {
        case DataSourceType::CSV:
            return CSVReader::read(
                path,
                table,
                errorMessage);

        case DataSourceType::Excel:
            return ExcelReader::read(
                path,
                table,
                errorMessage);

        case DataSourceType::Unknown:
            errorMessage =
                "Unsupported file format. "
                "Supported formats are .csv and .xlsx.";

            return false;
    }

    errorMessage =
        "Unsupported data source type.";

    return false;
}

} // namespace sdv
