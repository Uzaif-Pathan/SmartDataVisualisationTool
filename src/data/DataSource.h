#ifndef SMARTDATAVISUALISATION_DATASOURCE_H
#define SMARTDATAVISUALISATION_DATASOURCE_H

#include <string>

namespace sdv {

enum class DataSourceType {
    Unknown,
    CSV,
    Excel
};

class DataSource {
public:
    DataSource();
    explicit DataSource(const std::string& path);

    const std::string& path() const;
    void setPath(const std::string& path);

    DataSourceType type() const;
    void setType(DataSourceType type);

    bool exists() const;

private:
    std::string path_;
    DataSourceType type_;
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_DATASOURCE_H
