#ifndef SMARTDATAVISUALISATION_PATHUTILS_H
#define SMARTDATAVISUALISATION_PATHUTILS_H

#include <string>

namespace sdv::utils {

std::string joinPath(const std::string& first,
                     const std::string& second);

std::string getParentPath(const std::string& path);

std::string getAbsolutePath(const std::string& path);

std::string getHomeDirectory();

std::string getApplicationDataDirectory();

std::string getProjectsDirectory();

} // namespace sdv::utils

#endif // SMARTDATAVISUALISATION_PATHUTILS_H
