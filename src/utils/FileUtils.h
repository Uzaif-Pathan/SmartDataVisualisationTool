#ifndef SMARTDATAVISUALISATION_FILEUTILS_H
#define SMARTDATAVISUALISATION_FILEUTILS_H

#include <string>
#include <vector>

namespace sdv::utils {

bool fileExists(const std::string& path);
bool directoryExists(const std::string& path);

bool createDirectory(const std::string& path);

std::string readTextFile(const std::string& path);
bool writeTextFile(const std::string& path, const std::string& content);

std::string getFileExtension(const std::string& path);
std::string getFileName(const std::string& path);

std::vector<std::string> listFiles(const std::string& directory);

} // namespace sdv::utils

#endif // SMARTDATAVISUALISATION_FILEUTILS_H

