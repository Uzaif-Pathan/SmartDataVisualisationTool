#include "aggregation/TimeAggregation.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

struct Civil {
    long long year;
    int month;
    int day;
};

// Calendar date -> days since 1970-01-01 (Howard Hinnant's algorithm).
long long daysFromCivil(long long y, long long m, long long d) {
    if (m <= 2) y -= 1;
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const long long yoe = y - era * 400;
    const long long doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    const long long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

// Days since 1970-01-01 -> calendar date.
Civil civilFromDays(long long z) {
    z += 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const long long doe = z - era * 146097;
    const long long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const long long y = yoe + era * 400;
    const long long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const long long mp = (5 * doy + 2) / 153;
    const long long d = doy - (153 * mp + 2) / 5 + 1;
    const long long m = mp < 10 ? mp + 3 : mp - 9;
    return {y + (m <= 2 ? 1 : 0), static_cast<int>(m), static_cast<int>(d)};
}

// Splits "a-b-c" into exactly three all-digit parts (each 1-4 characters).
bool splitThree(const std::string& s, char sep, std::string parts[3]) {
    size_t start = 0;
    for (int i = 0; i < 3; ++i) {
        const size_t end = (i < 2) ? s.find(sep, start) : s.size();
        if (end == std::string::npos) return false;
        parts[i] = s.substr(start, end - start);
        if (parts[i].empty() || parts[i].size() > 4) return false;
        for (char c : parts[i])
            if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        start = end + 1;
    }
    return true;
}

// The number that identifies which bucket a day falls in.
// Keys sort in chronological order.
long long bucketKey(long long days, TimeGranularity g) {
    switch (g) {
        case TimeGranularity::Daily:
            return days;
        case TimeGranularity::Weekly: {
            // 1970-01-01 was a Thursday, so (days + 3) % 7 gives Monday = 0.
            const long long weekday = ((days + 3) % 7 + 7) % 7;
            return days - weekday;   // the Monday of that week
        }
        case TimeGranularity::Monthly: {
            const Civil c = civilFromDays(days);
            return c.year * 12 + (c.month - 1);
        }
        case TimeGranularity::Yearly:
            return civilFromDays(days).year;
    }
    return days;
}

std::string bucketLabel(long long key, TimeGranularity g) {
    char buf[64] = {0};
    switch (g) {
        case TimeGranularity::Daily:
        case TimeGranularity::Weekly: {
            const Civil c = civilFromDays(key);
            std::snprintf(buf, sizeof buf, "%04lld-%02d-%02d", c.year, c.month, c.day);
            break;
        }
        case TimeGranularity::Monthly:
            std::snprintf(buf, sizeof buf, "%04lld-%02lld", key / 12, key % 12 + 1);
            break;
        case TimeGranularity::Yearly:
            std::snprintf(buf, sizeof buf, "%04lld", key);
            break;
    }
    return std::string(buf);
}

std::string explain(TimeGranularity g, long long span, bool forced) {
    if (forced) return "Time granularity was set manually to " + toString(g) + ".";
    std::string word;
    switch (g) {
        case TimeGranularity::Daily:   word = "daily"; break;
        case TimeGranularity::Weekly:  word = "weekly"; break;
        case TimeGranularity::Monthly: word = "monthly"; break;
        case TimeGranularity::Yearly:  word = "yearly"; break;
    }
    return "The data spans " + std::to_string(span) + " day(s), so " + word +
           " aggregation is used to keep the chart readable.";
}

}  // namespace

std::string toString(TimeGranularity granularity) {
    switch (granularity) {
        case TimeGranularity::Daily:   return "Daily";
        case TimeGranularity::Weekly:  return "Weekly";
        case TimeGranularity::Monthly: return "Monthly";
        case TimeGranularity::Yearly:  return "Yearly";
    }
    return "Unknown";
}

std::optional<long long> TimeAggregation::parseDate(const std::string& text) {
    // Trim spaces, then drop any time-of-day part.
    const size_t first = text.find_first_not_of(" \t");
    if (first == std::string::npos) return std::nullopt;
    const size_t last = text.find_last_not_of(" \t");
    std::string s = text.substr(first, last - first + 1);
    const size_t cut = s.find_first_of(" T");
    if (cut != std::string::npos) s = s.substr(0, cut);

    std::string p[3];
    long long year = 0, month = 0, day = 0;
    if (s.find('-') != std::string::npos) {            // YYYY-MM-DD
        if (!splitThree(s, '-', p) || p[0].size() != 4) return std::nullopt;
        year = std::stoi(p[0]);
        month = std::stoi(p[1]);
        day = std::stoi(p[2]);
    } else if (s.find('/') != std::string::npos) {     // DD/MM/YYYY
        if (!splitThree(s, '/', p) || p[2].size() != 4) return std::nullopt;
        day = std::stoi(p[0]);
        month = std::stoi(p[1]);
        year = std::stoi(p[2]);
    } else {
        return std::nullopt;
    }

    if (year < 1 || month < 1 || month > 12 || day < 1 || day > 31)
        return std::nullopt;

    // Convert there and back: impossible dates (like Feb 30) do not survive.
    const long long days = daysFromCivil(year, month, day);
    const Civil back = civilFromDays(days);
    if (back.year != year || back.month != month || back.day != day)
        return std::nullopt;
    return days;
}

TimeGranularity TimeAggregation::chooseGranularity(long long spanDays) {
    if (spanDays <= 31) return TimeGranularity::Daily;
    if (spanDays <= 180) return TimeGranularity::Weekly;
    if (spanDays <= 1095) return TimeGranularity::Monthly;
    return TimeGranularity::Yearly;
}

TimeAggregationResult TimeAggregation::aggregate(
    const DataTable& table,
    const std::string& dateColumn,
    const std::string& valueColumn,
    AggregationFunction func,
    std::optional<TimeGranularity> forced) {

    const int dateCol = table.columnIndex(dateColumn);
    if (dateCol < 0) throw std::invalid_argument("Column not found: " + dateColumn);
    const size_t dateIdx = static_cast<size_t>(dateCol);

    const bool countRows = valueColumn.empty();
    size_t valueIdx = 0;
    if (countRows) {
        if (func != AggregationFunction::Count)
            throw std::invalid_argument("A value column is required for " + toString(func));
    } else {
        const int valueCol = table.columnIndex(valueColumn);
        if (valueCol < 0) throw std::invalid_argument("Column not found: " + valueColumn);
        valueIdx = static_cast<size_t>(valueCol);
    }

    TimeAggregationResult out;
    out.result.groupColumn = dateColumn;
    out.result.valueColumn = valueColumn;
    out.result.function = func;

    // Pass 1: read every date and find the time span.
    std::vector<std::pair<size_t, long long>> dated;   // (row, day number)
    long long minDay = 0, maxDay = 0;
    for (size_t row = 0; row < table.rowCount(); ++row) {
        std::optional<long long> day;
        if (!table.isMissing(row, dateIdx))
            day = parseDate(table.getString(row, dateIdx));
        if (!day) { ++out.result.skippedRows; continue; }
        if (dated.empty()) {
            minDay = maxDay = *day;
        } else {
            minDay = std::min(minDay, *day);
            maxDay = std::max(maxDay, *day);
        }
        dated.emplace_back(row, *day);
    }

    if (dated.empty()) {
        out.reason = "No valid dates were found in column '" + dateColumn + "'.";
        return out;
    }

    out.spanDays = maxDay - minDay;
    out.granularity = forced ? *forced : chooseGranularity(out.spanDays);
    out.reason = explain(out.granularity, out.spanDays, forced.has_value());

    // Pass 2: put each value into its time bucket (std::map keeps them sorted).
    std::map<long long, std::vector<double>> buckets;
    for (const auto& item : dated) {
        double value = 1.0;   // when counting rows, every row counts as 1
        if (!countRows) {
            const auto v = table.getNumber(item.first, valueIdx);
            if (!v || !std::isfinite(*v)) { ++out.result.skippedRows; continue; }
            value = *v;
        }
        buckets[bucketKey(item.second, out.granularity)].push_back(value);
    }

    for (const auto& entry : buckets) {
        const auto agg = applyAggregation(func, entry.second);
        if (!agg) continue;
        out.result.labels.push_back(bucketLabel(entry.first, out.granularity));
        out.result.values.push_back(*agg);
    }
    return out;
}
