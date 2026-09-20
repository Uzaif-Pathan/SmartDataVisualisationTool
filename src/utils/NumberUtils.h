#ifndef SMARTDATAVISUALISATION_NUMBERUTILS_H
#define SMARTDATAVISUALISATION_NUMBERUTILS_H

#include <string>

namespace sdv::utils {

bool isNumber(const std::string& value);

double toDouble(const std::string& value,
                bool* success = nullptr);

std::string formatNumber(double value,
                         int decimalPlaces = 2);

} // namespace sdv::utils

#endif // SMARTDATAVISUALISATION_NUMBERUTILS_H
