#ifndef SMARTDATAVISUALISATION_DATEUTILS_H
#define SMARTDATAVISUALISATION_DATEUTILS_H

#include <string>

namespace sdv::utils {

bool isValidDate(const std::string& value);

bool isValidDateTime(const std::string& value);

std::string normalizeDate(const std::string& value);

} // namespace sdv::utils

#endif // SMARTDATAVISUALISATION_DATEUTILS_H

