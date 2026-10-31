#pragma once
#include <cstddef>
#include <optional>
#include <string>

#include "aggregation/AggregationFunction.h"
#include "aggregation/AggregationResult.h"
#include "data/DataTable.h"

enum class TimeGranularity { Daily, Weekly, Monthly, Yearly };

std::string toString(TimeGranularity granularity);

struct TimeAggregationResult {
    AggregationResult result;   // labels are sorted oldest -> newest
    TimeGranularity granularity = TimeGranularity::Daily;
    long long spanDays = 0;     // last valid date minus first valid date
    std::string reason;         // why this granularity was used (for the UI)
};

class TimeAggregation {
public:
    // Converts text such as "2026-09-01", "01/09/2026" (DD/MM/YYYY) or
    // "2026-09-01 14:30:00" into a day number (days since 1970-01-01).
    // The time of day is ignored.  Returns nothing for invalid dates
    // such as "not-a-date" or "2026-02-30".
    static std::optional<long long> parseDate(const std::string& text);

    // Picks the bucket size from the date span:
    //   <= 31 days -> Daily, <= 180 -> Weekly, <= 1095 -> Monthly, else Yearly.
    static TimeGranularity chooseGranularity(long long spanDays);

    // Aggregates valueColumn by time period.
    //  - If valueColumn is empty, func must be Count (counts rows per period).
    //  - If `forced` has a value, it is used instead of the automatic choice.
    //  - Rows with an invalid/missing date or an unusable number are skipped
    //    and counted in result.skippedRows.
    // Throws std::invalid_argument for unknown columns or a bad function choice.
    static TimeAggregationResult aggregate(
        const DataTable& table,
        const std::string& dateColumn,
        const std::string& valueColumn,
        AggregationFunction func,
        std::optional<TimeGranularity> forced = std::nullopt);
};
