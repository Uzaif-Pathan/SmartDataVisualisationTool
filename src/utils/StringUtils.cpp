#include "StringUtils.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace sdv::utils {

std::string trim(const std::string& value) {
    const auto begin = std::find_if(
        value.begin(),
        value.end(),
        [](unsigned char character) {
            return !std::isspace(character);
        });

    const auto end = std::find_if(
        value.rbegin(),
        value.rend(),
        [](unsigned char character) {
            return !std::isspace(character);
        }).base();

    if (begin >= end) {
        return {};
    }

    return std::string(begin, end);
}

std::string toLower(const std::string& value) {
    std::string result = value;

    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::tolower(character));
        });

    return result;
}

std::string toUpper(const std::string& value) {
    std::string result = value;

    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::toupper(character));
        });

    return result;
}

bool startsWith(const std::string& value,
                const std::string& prefix) {
    return value.size() >= prefix.size() &&
           value.compare(0, prefix.size(), prefix) == 0;
}

bool endsWith(const std::string& value,
              const std::string& suffix) {
    return value.size() >= suffix.size() &&
           value.compare(
               value.size() - suffix.size(),
               suffix.size(),
               suffix) == 0;
}

std::vector<std::string> split(const std::string& value,
                               char delimiter) {
    std::vector<std::string> result;
    std::stringstream stream(value);
    std::string item;

    while (std::getline(stream, item, delimiter)) {
        result.push_back(item);
    }

    return result;
}

std::string join(const std::vector<std::string>& values,
                 const std::string& separator) {
    std::string result;

    for (std::size_t i = 0; i < values.size(); ++i) {
        result += values[i];

        if (i + 1 < values.size()) {
            result += separator;
        }
    }

    return result;
}

bool isEmpty(const std::string& value) {
    return trim(value).empty();
}

} // namespace sdv::utils
