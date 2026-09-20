#include "NumberUtils.h"

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace sdv::utils {

bool isNumber(const std::string& value) {
    try {
        std::size_t position = 0;

        std::stod(value, &position);

        return position == value.size();
    }
    catch (...) {
        return false;
    }
}

double toDouble(const std::string& value,
                bool* success) {
    try {
        std::size_t position = 0;

        const double result = std::stod(value, &position);

        const bool valid = position == value.size();

        if (success != nullptr) {
            *success = valid;
        }

        return valid ? result : 0.0;
    }
    catch (...) {
        if (success != nullptr) {
            *success = false;
        }

        return 0.0;
    }
}

std::string formatNumber(double value,
                         int decimalPlaces) {
    std::ostringstream stream;

    stream << std::fixed
           << std::setprecision(decimalPlaces)
           << value;

    return stream.str();
}

} // namespace sdv::utils
