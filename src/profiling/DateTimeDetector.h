#pragma once

#include "../data/DataColumn.h"

namespace sdv {

// Detects whether a column's values look like dates/timestamps.
// Recognizes: YYYY-MM-DD, DD/MM/YYYY, YYYY-MM-DD HH:MM:SS.
class DateTimeDetector {
public:
    // Fraction (0..1) of non-missing values that match a supported
    // date/time format.
    double dateLikeRatio(const DataColumn& column) const;

    // Convenience: true if dateLikeRatio() clears the given threshold
    // (default 0.6, i.e. the column is "mostly" dates).
    bool isLikelyDateTime(const DataColumn& column, double threshold = 0.6) const;
};

} // namespace sdv
