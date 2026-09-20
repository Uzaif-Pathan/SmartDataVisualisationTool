#ifndef SMARTDATAVISUALISATION_DATALOADER_H
#define SMARTDATAVISUALISATION_DATALOADER_H

#include "DataTable.h"

#include <string>

namespace sdv {

class DataLoader {
public:
    static bool load(const std::string& path,
                     DataTable& table,
                     std::string& errorMessage);
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_DATALOADER_H
