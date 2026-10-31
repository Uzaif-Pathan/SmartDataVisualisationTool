#include "RecommendationContext.h"
#include "../profiling/ColumnProfiler.h"

namespace sdv {

RecommendationContext buildRecommendationContext(const DataTable& table) {
    ColumnProfiler profiler;
    return RecommendationContext(table, profiler.profileTable(table));
}

} // namespace sdv
