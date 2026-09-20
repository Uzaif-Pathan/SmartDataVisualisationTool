#ifndef SMARTDATAVISUALISATION_CSVWRITER_H
#define SMARTDATAVISUALISATION_CSVWRITER_H

#include "DataTable.h"

#include <string>

namespace sdv {

class CSVWriter {
public:
    static bool write(
        const std::string& path,
        const DataTable& table,
        std::string& errorMessage);
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_CSVWRITER_H
