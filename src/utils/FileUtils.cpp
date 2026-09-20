#include "FileUtils.h"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

namespace sdv::utils {

bool fileExists(const std::string& path) {
    return fs::exists(path) && fs::is_regular_file(path);
}

bool directoryExists(const std::string& path) {
    return fs::exists(path) && fs::is_directory(path);
}

bool createDirectory(const std::string& path) {
    if (path.empty()) {
        return false;
    }

    std::error_code error;
    fs::create_directories(path, error);

    return !error && directoryExists(path);
}

std::string readTextFile(const std::string& path) {
    std::ifstream file(path);

    if (!file.is_open()) {
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

bool writeTextFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);

    if (!file.is_open()) {
        return false;
    }

    file << content;

    return file.good();
}

std::string getFileExtension(const std::string& path) {
    return fs::path(path).extension().string();
}

std::string getFileName(const std::string& path) {
    return fs::path(path).filename().string();
}

std::vector<std::string> listFiles(const std::string& directory) {
    std::vector<std::string> files;

    if (!directoryExists(directory)) {
        return files;
    }

    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            files.push_back(entry.path().string());
        }
    }

    return files;
}

} // namespace sdv::utils
