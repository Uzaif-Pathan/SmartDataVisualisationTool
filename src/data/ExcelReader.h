#ifndef SMARTDATAVISUALISATION_EXCELREADER_H
#define SMARTDATAVISUALISATION_EXCELREADER_H

#include "DataTable.h"

#include <string>

namespace sdv {

class ExcelReader {
public:
    static bool read(const std::string& path,
                     DataTable& table,
                     std::string& errorMessage);
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_EXCELREADER_H
