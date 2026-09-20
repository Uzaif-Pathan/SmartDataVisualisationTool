#include "PathUtils.h"

#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

namespace sdv::utils {

std::string joinPath(const std::string& first,
                     const std::string& second) {
    return (fs::path(first) / fs::path(second)).string();
}

std::string getParentPath(const std::string& path) {
    return fs::path(path).parent_path().string();
}

std::string getAbsolutePath(const std::string& path) {
    std::error_code error;

    const auto absolute = fs::absolute(path, error);

    if (error) {
        return path;
    }

    return absolute.string();
}

std::string getHomeDirectory() {
    const char* home = std::getenv("HOME");

    if (home == nullptr) {
        return {};
    }

    return home;
}

std::string getApplicationDataDirectory() {
    const std::string home = getHomeDirectory();

    if (home.empty()) {
        return {};
    }

    return joinPath(home, ".smart-data-visualisation");
}

std::string getProjectsDirectory() {
    return joinPath(getApplicationDataDirectory(), "projects");
}

} // namespace sdv::utils
