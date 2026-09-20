#ifndef SMARTDATAVISUALISATION_STRINGUTILS_H
#define SMARTDATAVISUALISATION_STRINGUTILS_H

#include <string>
#include <vector>

namespace sdv::utils {

std::string trim(const std::string& value);

std::string toLower(const std::string& value);
std::string toUpper(const std::string& value);

bool startsWith(const std::string& value,
                const std::string& prefix);

bool endsWith(const std::string& value,
              const std::string& suffix);

std::vector<std::string> split(const std::string& value,
                               char delimiter);

std::string join(const std::vector<std::string>& values,
                 const std::string& separator);

bool isEmpty(const std::string& value);

} // namespace sdv::utils

#endif // SMARTDATAVISUALISATION_STRINGUTILS_H
