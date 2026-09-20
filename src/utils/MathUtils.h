#ifndef SMARTDATAVISUALISATION_MATHUTILS_H
#define SMARTDATAVISUALISATION_MATHUTILS_H

#include <vector>

namespace sdv::utils {

double clamp(double value,
             double minimum,
             double maximum);

bool approximatelyEqual(double first,
                        double second,
                        double epsilon = 1e-9);

double linearMap(double value,
                 double inputMinimum,
                 double inputMaximum,
                 double outputMinimum,
                 double outputMaximum);

double mean(const std::vector<double>& values);

double sum(const std::vector<double>& values);

double minimum(const std::vector<double>& values);

double maximum(const std::vector<double>& values);

} // namespace sdv::utils

#endif // SMARTDATAVISUALISATION_MATHUTILS_H
