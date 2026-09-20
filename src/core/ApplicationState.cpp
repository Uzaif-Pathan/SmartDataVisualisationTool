#include "ApplicationState.h"

namespace sdv {

ApplicationState::ApplicationState()
    : currentPage_(ApplicationPage::Home),
      currentProjectPath_(),
      currentDatasetPath_() {
}

ApplicationPage ApplicationState::currentPage() const {
    return currentPage_;
}

void ApplicationState::setCurrentPage(ApplicationPage page) {
    currentPage_ = page;
}

const std::string& ApplicationState::currentProjectPath() const {
    return currentProjectPath_;
}

void ApplicationState::setCurrentProjectPath(const std::string& path) {
    currentProjectPath_ = path;
}

const std::string& ApplicationState::currentDatasetPath() const {
    return currentDatasetPath_;
}

void ApplicationState::setCurrentDatasetPath(const std::string& path) {
    currentDatasetPath_ = path;
}

bool ApplicationState::hasDataset() const {
    return !currentDatasetPath_.empty();
}

bool ApplicationState::hasProject() const {
    return !currentProjectPath_.empty();
}

void ApplicationState::clear() {
    currentPage_ = ApplicationPage::Home;
    currentProjectPath_.clear();
    currentDatasetPath_.clear();
}

} // namespace sdv
