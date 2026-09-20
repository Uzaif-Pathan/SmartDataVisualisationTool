#include "DataSource.h"

#include "utils/FileUtils.h"

namespace sdv {

DataSource::DataSource()
    : path_(),
      type_(DataSourceType::Unknown) {
}

DataSource::DataSource(const std::string& path)
    : path_(path),
      type_(DataSourceType::Unknown) {
}

const std::string& DataSource::path() const {
    return path_;
}

void DataSource::setPath(const std::string& path) {
    path_ = path;
}

DataSourceType DataSource::type() const {
    return type_;
}

void DataSource::setType(DataSourceType type) {
    type_ = type;
}

bool DataSource::exists() const {
    return utils::fileExists(path_);
}

} // namespace sdv
