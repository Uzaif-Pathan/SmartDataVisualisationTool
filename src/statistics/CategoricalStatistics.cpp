#include "CategoricalStatistics.h"

#include <algorithm>
#include <map>

namespace sdv {

CategoricalStatisticsResult
CategoricalStatistics::calculate(
    const DataColumn& column) {

    CategoricalStatisticsResult result;

    std::map<std::string, std::size_t> frequencyMap;

    for (const DataValue& value : column.values()) {
        if (value.isEmpty()) {
            ++result.missingCount;
            continue;
        }

        ++result.count;
        ++frequencyMap[value.toString()];
    }

    result.uniqueCount = frequencyMap.size();

    if (result.count == 0) {
        return result;
    }

    result.frequencies.reserve(
        frequencyMap.size());

    for (const auto& entry : frequencyMap) {
        CategoryFrequency frequency;

        frequency.value = entry.first;
        frequency.count = entry.second;

        frequency.percentage =
            (static_cast<double>(entry.second) /
             static_cast<double>(result.count)) *
            100.0;

        result.frequencies.push_back(frequency);
    }

    std::sort(
        result.frequencies.begin(),
        result.frequencies.end(),
        [](const CategoryFrequency& first,
           const CategoryFrequency& second) {

            if (first.count != second.count) {
                return first.count > second.count;
            }

            return first.value < second.value;
        });

    if (!result.frequencies.empty()) {
        result.mostFrequentValue =
            result.frequencies.front().value;

        result.mostFrequentCount =
            result.frequencies.front().count;

        result.leastFrequentValue =
            result.frequencies.back().value;

        result.leastFrequentCount =
            result.frequencies.back().count;
    }

    result.hasValidStatistics = true;

    return result;
}

} // namespace sdv
