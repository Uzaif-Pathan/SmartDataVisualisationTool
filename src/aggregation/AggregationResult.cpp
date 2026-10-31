#include "aggregation/AggregationResult.h"

#include <iomanip>
#include <sstream>

std::string AggregationResult::toText() const {
    std::ostringstream out;
    out << std::setprecision(10);
    out << toString(function) << " of "
        << (valueColumn.empty() ? "rows" : valueColumn)
        << " by " << groupColumn << "\n";
    for (size_t i = 0; i < labels.size(); ++i)
        out << "  " << labels[i] << " -> " << values[i] << "\n";
    if (skippedRows > 0)
        out << "  (" << skippedRows << " row(s) skipped: missing/invalid values)\n";
    return out.str();
}
