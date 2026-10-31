#include "VariablePairAnalyzer.h"

namespace sdv {

PairRelationship VariablePairAnalyzer::classify(const ColumnProfile& a,
                                                  const ColumnProfile& b) const {
    // Identifiers (Customer_Name-as-key, Customer_ID, ...) never make
    // a statistically meaningful axis — reject immediately regardless
    // of the other column's type. This is what keeps combinations
    // like "Customer_Name x Sales" or "Customer_ID x Quantity" out of
    // the recommendation list.
    if (a.type == DetectedType::Identifier || b.type == DetectedType::Identifier) {
        return PairRelationship::InvolvesIdentifier;
    }
    // A fully constant column (every value the same) also carries no
    // information for a two-variable chart.
    if (a.isConstant || b.isConstant) {
        return PairRelationship::Unsupported;
    }

    bool aDate = (a.type == DetectedType::DateTime);
    bool bDate = (b.type == DetectedType::DateTime);
    bool aNumeric = (a.type == DetectedType::Numeric);
    bool bNumeric = (b.type == DetectedType::Numeric);
    bool aCategorical = (a.type == DetectedType::Categorical || a.type == DetectedType::Boolean);
    bool bCategorical = (b.type == DetectedType::Categorical || b.type == DetectedType::Boolean);

    if ((aDate && bNumeric) || (bDate && aNumeric)) {
        return PairRelationship::DateTimeVsNumeric;
    }
    if ((aCategorical && bNumeric) || (bCategorical && aNumeric)) {
        return PairRelationship::CategoricalVsNumeric;
    }
    if (aNumeric && bNumeric) {
        return PairRelationship::NumericVsNumeric;
    }
    if (aCategorical && bCategorical) {
        return PairRelationship::CategoricalVsCategorical;
    }

    return PairRelationship::Unsupported;
}

} // namespace sdv
