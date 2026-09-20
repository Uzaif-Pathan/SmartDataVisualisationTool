#include "MathUtils.h"

#include <algorithm>
#include <numeric>

namespace sdv::utils {

double clamp(double value,
             double minimumValue,
             double maximumValue) {
    return std::max(minimumValue,
                    std::min(value, maximumValue));
}

bool approximatelyEqual(double first,
                        double second,
                        double epsilon) {
    return std::abs(first - second) <= epsilon;
}

double linearMap(double value,
                 double inputMinimum,
                 double inputMaximum,
                 double outputMinimum,
                 double outputMaximum) {
    if (approximatelyEqual(inputMinimum, inputMaximum)) {
        return outputMinimum;
    }

    const double ratio =
        (value - inputMinimum) /
        (inputMaximum - inputMinimum);

    return outputMinimum +
           ratio * (outputMaximum - outputMinimum);
}

double mean(const std::vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }

    return sum(values) /
           static_cast<double>(values.size());
}

double sum(const std::vector<double>& values) {
    return std::accumulate(
        values.begin(),
        values.end(),
        0.0);
}

double minimum(const std::vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }

    return *std::min_element(
        values.begin(),
        values.end());
}

double maximum(const std::vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }

    return *std::max_element(
        values.begin(),
        values.end());
}

} // namespace sdv::utils
