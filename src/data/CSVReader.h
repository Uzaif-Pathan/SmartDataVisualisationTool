#ifndef SMARTDATAVISUALISATION_CSVREADER_H
#define SMARTDATAVISUALISATION_CSVREADER_H

#include "DataTable.h"

#include <string>

namespace sdv {

class CSVReader {
public:
    static bool read(
        const std::string& path,
        DataTable& table,
        std::string& errorMessage);

private:
    static DataValue convertValue(
        const std::string& value);
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_CSVREADER_H
