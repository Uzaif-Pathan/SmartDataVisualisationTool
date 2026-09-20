#include <iostream>

#include "core/Constants.h"
#include "data/DataLoader.h"

int main() {
    std::cout
        << sdv::constants::APPLICATION_NAME
        << " v"
        << sdv::constants::APPLICATION_VERSION
        << '\n';

    sdv::DataTable table;
    std::string errorMessage;

    const std::string datasetPath =
    "sample_data/test_excel.xlsx";
    if (!sdv::DataLoader::load(
            datasetPath,
            table,
            errorMessage)) {

        std::cerr
            << "Dataset loading failed: "
            << errorMessage
            << '\n';

        return 1;
    }

    std::cout
        << "\nDataset loaded successfully.\n";

    std::cout
        << "Columns: "
        << table.columnCount()
        << '\n';

    std::cout
        << "Rows: "
        << table.rowCount()
        << '\n';

    std::cout << "\nColumns:\n";

    for (const auto& column : table.columns()) {
        std::cout
            << "- "
            << column.name()
            << " | values: "
            << column.size()
            << " | missing: "
            << column.missingCount()
            << " | unique: "
            << column.uniqueCount()
            << '\n';
    }

    return 0;
}
