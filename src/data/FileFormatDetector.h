#ifndef SMARTDATAVISUALISATION_FILEFORMATDETECTOR_H
#define SMARTDATAVISUALISATION_FILEFORMATDETECTOR_H

#include "DataSource.h"

#include <string>

namespace sdv {

class FileFormatDetector {
public:
    static DataSourceType detect(const std::string& path);

    static bool isSupported(const std::string& path);

    static std::string extension(const std::string& path);
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_FILEFORMATDETECTOR_H

