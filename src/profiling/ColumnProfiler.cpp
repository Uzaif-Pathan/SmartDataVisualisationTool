#include "ColumnProfiler.h"
#include "DataTypeDetector.h"
#include "CardinalityAnalyzer.h"
#include "IdentifierDetector.h"
#include "MissingnessAnalyzer.h"
#include "VariationAnalyzer.h"
#include "CategoricalAnalyzer.h"

namespace sdv {

ColumnProfile ColumnProfiler::profileColumn(const DataColumn& column) const {
    ColumnProfile profile;
    profile.columnName = column.name();

    MissingnessAnalyzer missingness;
    MissingnessInfo missInfo = missingness.analyze(column);
    profile.totalCount = missInfo.totalCount;
    profile.missingCount = missInfo.missingCount;
    profile.missingPercent = missInfo.missingPercent;

    CardinalityAnalyzer cardinality;
    CardinalityInfo cardInfo = cardinality.analyze(column);
    profile.uniqueCount = cardInfo.uniqueCount;
    profile.uniqueRatio = cardInfo.uniqueRatio;
    profile.isConstant = (cardInfo.nonMissingCount > 0 && cardInfo.uniqueCount <= 1);

    IdentifierDetector identifierDetector;
    profile.identifierScore = identifierDetector.identifierScore(column);
    profile.isLikelyIdentifier = identifierDetector.isLikelyIdentifier(column);

    VariationAnalyzer variationAnalyzer;
    profile.variation = variationAnalyzer.classify(column);

    DataTypeDetector typeDetector;
    profile.type = typeDetector.detect(column);

    if (profile.type == DetectedType::Categorical) {
        CategoricalAnalyzer categoricalAnalyzer;
        profile.topCategories = categoricalAnalyzer.topCategories(column, kTopCategoriesLimit);
    }

    return profile;
}

std::vector<ColumnProfile> ColumnProfiler::profileTable(const DataTable& table) const {
    std::vector<ColumnProfile> profiles;
    profiles.reserve(table.columnCount());
    for (size_t c = 0; c < table.columnCount(); ++c) {
        profiles.push_back(profileColumn(table.getColumn(c)));
    }
    return profiles;
}

} // namespace sdv
