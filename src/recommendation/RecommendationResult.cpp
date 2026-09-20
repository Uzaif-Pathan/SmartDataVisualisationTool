#include "RecommendationResult.h"
#include <sstream>
#include <iomanip>

namespace sdv {

std::string RecommendationResult::describe() const {
    std::ostringstream oss;
    oss << plotTypeToString(plotType) << ": " << xColumn;
    if (!yColumn.empty()) oss << " x " << yColumn;
    oss << " (score " << std::fixed << std::setprecision(2) << suitabilityScore << ") — "
        << reason;
    return oss.str();
}

} // namespace sdv
