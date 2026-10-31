#pragma once

#include <string>
#include <vector>
#include <utility>

namespace sdv {

enum class DetectedType { Numeric, Categorical, DateTime, Boolean, Identifier, Unknown };
enum class VariationLevel { Constant, LowVariation, Normal, HighVariation };

std::string detectedTypeToString(DetectedType t);
std::string variationLevelToString(VariationLevel v);

// Everything the rest of the pipeline (validation report aside) needs
// to know about a single column, produced by ColumnProfiler.
struct ColumnProfile {
    std::string columnName;
    DetectedType type = DetectedType::Unknown;

    size_t totalCount = 0;
    size_t missingCount = 0;
    double missingPercent = 0.0;

    size_t uniqueCount = 0;
    double uniqueRatio = 0.0;      // uniqueCount / non-missing count

    bool isConstant = false;
    bool isLikelyIdentifier = false;
    double identifierScore = 0.0;  // 0..1 confidence

    VariationLevel variation = VariationLevel::Normal;

    // Only populated for Categorical columns: value -> frequency.
    std::vector<std::pair<std::string, size_t>> topCategories;
};

} // namespace sdv
