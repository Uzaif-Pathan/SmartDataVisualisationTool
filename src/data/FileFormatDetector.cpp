#include "FileFormatDetector.h"

#include "utils/StringUtils.h"

#include <filesystem>

namespace sdv {

DataSourceType FileFormatDetector::detect(
    const std::string& path) {

    const std::string ext =
        utils::toLower(
            std::filesystem::path(path)
                .extension()
                .string());

    if (ext == ".csv") {
        return DataSourceType::CSV;
    }

    if (ext == ".xlsx") {
        return DataSourceType::Excel;
    }

    return DataSourceType::Unknown;
}

bool FileFormatDetector::isSupported(
    const std::string& path) {

    return detect(path) !=
           DataSourceType::Unknown;
}

std::string FileFormatDetector::extension(
    const std::string& path) {

    return std::filesystem::path(path)
        .extension()
        .string();
}

} // namespace sdv
