#ifndef SMARTDATAVISUALISATION_APPLICATIONSTATE_H
#define SMARTDATAVISUALISATION_APPLICATIONSTATE_H

#include "Enums.h"

#include <string>

namespace sdv {

class ApplicationState {
public:
    ApplicationState();

    ApplicationPage currentPage() const;
    void setCurrentPage(ApplicationPage page);

    const std::string& currentProjectPath() const;
    void setCurrentProjectPath(const std::string& path);

    const std::string& currentDatasetPath() const;
    void setCurrentDatasetPath(const std::string& path);

    bool hasDataset() const;
    bool hasProject() const;

    void clear();

private:
    ApplicationPage currentPage_;
    std::string currentProjectPath_;
    std::string currentDatasetPath_;
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_APPLICATIONSTATE_H
