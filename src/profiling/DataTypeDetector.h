#pragma once

#include "ColumnProfile.h"
#include "../data/DataColumn.h"

namespace sdv {

// Combines NumericAnalyzer, DateTimeDetector, IdentifierDetector and
// a boolean check into a single decision: what IS this column?
//
// Decision order matters:
//   1. Identifier wins first — a column of near-unique values is not
//      meaningfully "numeric" or "categorical" even if it happens to
//      contain numbers (e.g. Customer_ID).
//   2. Boolean (true/false, yes/no) — checked before numeric/date
//      since booleans could otherwise slip through as text.
//   3. DateTime — checked before numeric, since some date-only
//      substrings could otherwise parse as numbers (not the case with
//      our formats, but keeps precedence explicit and safe).
//   4. Numeric.
//   5. Categorical — anything left with low-to-moderate cardinality.
//   6. Unknown — high-cardinality free text that is not an identifier
//      (e.g. free-form notes/comments).
class DataTypeDetector {
public:
    DetectedType detect(const DataColumn& column) const;

private:
    bool isLikelyBoolean(const DataColumn& column) const;
};

} // namespace sdv
