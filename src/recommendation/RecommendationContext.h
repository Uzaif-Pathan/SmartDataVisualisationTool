#pragma once

#include <vector>
#include <string>
#include "../data/DataTable.h"
#include "../profiling/ColumnProfile.h"

namespace sdv {

// Everything the recommendation engine needs to reason about a
// dataset: the raw table (for row counts etc.) plus the column
// profiles already computed by Member 2's own ColumnProfiler. Kept as
// a separate struct (rather than passing DataTable and profiles
// around loosely) so the engine's public API stays simple and stable
// as the rule set grows.
struct RecommendationContext {
    const DataTable& table;
    std::vector<ColumnProfile> profiles;

    explicit RecommendationContext(const DataTable& t, std::vector<ColumnProfile> p)
        : table(t), profiles(std::move(p)) {}

    // Convenience: find a column's profile by name. Returns nullptr
    // if not found (should not normally happen if profiles were
    // built from the same table).
    const ColumnProfile* profileFor(const std::string& columnName) const {
        for (const auto& p : profiles) {
            if (p.columnName == columnName) return &p;
        }
        return nullptr;
    }
};

// Convenience factory: profiles the table with a fresh ColumnProfiler
// and wraps the result in a RecommendationContext. Declared here,
// defined in RecommendationContext.cpp.
RecommendationContext buildRecommendationContext(const DataTable& table);

} // namespace sdv
