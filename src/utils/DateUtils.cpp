#include "DateUtils.h"

#include <iomanip>
#include <sstream>

namespace sdv::utils {

bool isValidDate(const std::string& value) {
    std::tm time = {};
    std::istringstream stream(value);

    stream >> std::get_time(&time, "%Y-%m-%d");

    return !stream.fail() && stream.eof();
}

bool isValidDateTime(const std::string& value) {
    std::tm time = {};
    std::istringstream stream(value);

    stream >> std::get_time(&time, "%Y-%m-%d %H:%M:%S");

    return !stream.fail();
}

std::string normalizeDate(const std::string& value) {
    std::tm time = {};
    std::istringstream input(value);

    input >> std::get_time(&time, "%Y-%m-%d");

    if (input.fail()) {
        return {};
    }

    std::ostringstream output;
    output << std::put_time(&time, "%Y-%m-%d");

    return output.str();
}

} // namespace sdv::utils
