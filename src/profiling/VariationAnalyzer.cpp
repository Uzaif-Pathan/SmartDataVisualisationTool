#include "VariationAnalyzer.h"
#include "CardinalityAnalyzer.h"

namespace sdv {

VariationLevel VariationAnalyzer::classify(const DataColumn& column) const {
    CardinalityAnalyzer cardinality;
    CardinalityInfo info = cardinality.analyze(column);

    if (info.nonMissingCount == 0) return VariationLevel::Constant;
    if (info.uniqueCount <= 1) return VariationLevel::Constant;
    if (info.uniqueRatio <= kLowVariationThreshold) return VariationLevel::LowVariation;
    if (info.uniqueRatio >= kHighVariationThreshold) return VariationLevel::HighVariation;
    return VariationLevel::Normal;
}

} // namespace sdv
