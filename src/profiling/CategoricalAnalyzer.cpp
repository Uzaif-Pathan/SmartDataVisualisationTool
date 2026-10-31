#include "CategoricalAnalyzer.h"
#include <unordered_map>
#include <algorithm>

namespace sdv {

std::vector<std::pair<std::string, size_t>> CategoricalAnalyzer::frequencyTable(
    const DataColumn& column) const {
    std::unordered_map<std::string, size_t> counts;
    for (size_t i = 0; i < column.size(); ++i) {
        const DataValue& v = column.at(i);
        if (v.isMissing() || (v.isString() && v.asString().empty())) continue;
        ++counts[v.asString()];
    }

    std::vector<std::pair<std::string, size_t>> table(counts.begin(), counts.end());
    std::sort(table.begin(), table.end(), [](const auto& a, const auto& b) {
        if (a.second != b.second) return a.second > b.second; // descending frequency
        return a.first < b.first;                              // stable tie-break
    });
    return table;
}

std::vector<std::pair<std::string, size_t>> CategoricalAnalyzer::topCategories(
    const DataColumn& column, size_t topN) const {
    auto table = frequencyTable(column);
    if (table.size() > topN) table.resize(topN);
    return table;
}

} // namespace sdv
