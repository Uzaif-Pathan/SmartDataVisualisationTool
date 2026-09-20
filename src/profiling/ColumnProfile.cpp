#include "ColumnProfile.h"

namespace sdv {

std::string detectedTypeToString(DetectedType t) {
    switch (t) {
        case DetectedType::Numeric:     return "Numeric";
        case DetectedType::Categorical: return "Categorical";
        case DetectedType::DateTime:    return "DateTime";
        case DetectedType::Boolean:     return "Boolean";
        case DetectedType::Identifier:  return "Identifier";
        case DetectedType::Unknown:
        default:                        return "Unknown";
    }
}

std::string variationLevelToString(VariationLevel v) {
    switch (v) {
        case VariationLevel::Constant:      return "Constant";
        case VariationLevel::LowVariation:  return "LowVariation";
        case VariationLevel::Normal:        return "Normal";
        case VariationLevel::HighVariation: return "HighVariation";
    }
    return "Normal";
}

} // namespace sdv
