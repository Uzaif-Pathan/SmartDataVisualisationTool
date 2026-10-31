#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

#include "aggregation/Aggregator.h"
#include "aggregation/DerivedMetrics.h"
#include "aggregation/TimeAggregation.h"
#include "data/DataTable.h"

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

static bool near(double a, double b) { return std::fabs(a - b) < 1e-6; }

static double valueOf(const AggregationResult& r, const std::string& label) {
    for (size_t i = 0; i < r.labels.size(); ++i)
        if (r.labels[i] == label) return r.values[i];
    return std::nan("");
}

int main() {
    try {
        // ---- Date parsing ----
        auto iso   = TimeAggregation::parseDate("2026-09-01");
        auto dmy   = TimeAggregation::parseDate("01/09/2026");
        auto stamp = TimeAggregation::parseDate("2026-09-01 14:30:00");
        auto epoch = TimeAggregation::parseDate("1970-01-01");
        auto week  = TimeAggregation::parseDate("1970-01-08");
        check(iso.has_value(), "ISO date 2026-09-01 parses");
        check(iso && dmy && *iso == *dmy, "01/09/2026 is read as DD/MM/YYYY = 2026-09-01");
        check(iso && stamp && *iso == *stamp, "time of day is ignored");
        check(epoch && *epoch == 0, "1970-01-01 is day 0");
        check(week && *week == 7, "1970-01-08 is day 7");
        check(!TimeAggregation::parseDate("not-a-date").has_value(), "'not-a-date' rejected");
        check(!TimeAggregation::parseDate("2026-02-30").has_value(), "Feb 30 rejected");
        check(!TimeAggregation::parseDate("2026-13-01").has_value(), "month 13 rejected");
        check(!TimeAggregation::parseDate("").has_value(), "empty text rejected");
        check(TimeAggregation::parseDate("2028-02-29").has_value(), "leap day 2028-02-29 accepted");
        check(!TimeAggregation::parseDate("2026-02-29").has_value(), "2026-02-29 rejected (not a leap year)");

        // ---- Granularity thresholds ----
        using G = TimeGranularity;
        check(TimeAggregation::chooseGranularity(0) == G::Daily, "span 0 -> Daily");
        check(TimeAggregation::chooseGranularity(31) == G::Daily, "span 31 -> Daily");
        check(TimeAggregation::chooseGranularity(32) == G::Weekly, "span 32 -> Weekly");
        check(TimeAggregation::chooseGranularity(180) == G::Weekly, "span 180 -> Weekly");
        check(TimeAggregation::chooseGranularity(181) == G::Monthly, "span 181 -> Monthly");
        check(TimeAggregation::chooseGranularity(1095) == G::Monthly, "span 1095 -> Monthly");
        check(TimeAggregation::chooseGranularity(1096) == G::Yearly, "span 1096 -> Yearly");

        // ---- Daily aggregation ----
        DataTable ts = DataTable::fromCsv("sample_data/time_series.csv");
        auto daily = TimeAggregation::aggregate(ts, "Date", "Sales", AggregationFunction::Sum);
        std::cout << "\n" << daily.reason << "\n" << daily.result.toText() << "\n";
        check(daily.granularity == G::Daily, "time_series.csv -> Daily");
        check(daily.spanDays == 7, "span is 7 days");
        check(daily.result.size() == 8, "8 daily buckets");
        check(std::is_sorted(daily.result.labels.begin(), daily.result.labels.end()),
              "buckets are in chronological order");
        check(near(valueOf(daily.result, "2026-09-01"), 80000), "09-01 = 80000");
        check(near(valueOf(daily.result, "2026-09-03"), 52000), "09-03 = 52000");

        // ---- Weekly / monthly / yearly ----
        DataTable wk = DataTable::fromCsv("sample_data/weekly_test.csv");
        auto weekly = TimeAggregation::aggregate(wk, "Date", "Sales", AggregationFunction::Sum);
        std::cout << "\n" << weekly.reason << "\n" << weekly.result.toText() << "\n";
        check(weekly.granularity == G::Weekly, "60-day span -> Weekly");
        check(weekly.spanDays == 60, "span is 60 days");
        check(weekly.result.size() == 5, "5 weekly buckets");
        check(near(valueOf(weekly.result, "2026-06-29"), 300), "week of Mon 2026-06-29 = 300");
        check(near(valueOf(weekly.result, "2026-07-06"), 300), "week of Mon 2026-07-06 = 300");
        check(near(valueOf(weekly.result, "2026-08-24"), 600), "Sunday 08-30 falls in week of Mon 08-24");

        auto monthly = TimeAggregation::aggregate(wk, "Date", "Sales",
                                                  AggregationFunction::Sum, G::Monthly);
        check(monthly.result.size() == 2, "forced Monthly gives 2 buckets");
        check(near(valueOf(monthly.result, "2026-07"), 1000), "July = 1000");
        check(near(valueOf(monthly.result, "2026-08"), 1100), "August = 1100");

        auto yearly = TimeAggregation::aggregate(wk, "Date", "Sales",
                                                 AggregationFunction::Sum, G::Yearly);
        check(yearly.result.size() == 1 && near(valueOf(yearly.result, "2026"), 2100),
              "forced Yearly: 2026 = 2100");

        // ---- Bad data is reported, not hidden ----
        DataTable bad = DataTable::fromCsv("sample_data/invalid_dates.csv");
        auto badRes = TimeAggregation::aggregate(bad, "Date", "Sales", AggregationFunction::Sum);
        check(badRes.result.size() == 2, "invalid_dates.csv gives 2 usable days");
        check(badRes.result.skippedRows == 2, "2 rows reported as skipped");
        check(near(valueOf(badRes.result, "2026-09-02"), 300), "09-02 = 300");

        // ---- Counting rows per period ----
        auto counts = TimeAggregation::aggregate(ts, "Date", "", AggregationFunction::Count);
        check(near(valueOf(counts.result, "2026-09-01"), 2), "2 rows on 09-01");
        check(near(valueOf(counts.result, "2026-09-02"), 1), "1 row on 09-02");

        bool threw = false;
        try { TimeAggregation::aggregate(ts, "Date", "", AggregationFunction::Sum); }
        catch (const std::invalid_argument&) { threw = true; }
        check(threw, "Sum without a value column throws");

        // ---- Derived metrics ----
        DataTable sales = DataTable::fromCsv("sample_data/sales.csv");
        auto byProduct = Aggregator::aggregate(sales, "Product", "Sales", AggregationFunction::Sum);

        auto pct = DerivedMetrics::toPercentages(byProduct);
        check(pct.has_value(), "percentages computed");
        if (pct) {
            check(near(valueOf(*pct, "Laptop"), 205000.0 / 376000.0 * 100.0),
                  "Laptop share is correct");
            double total = 0;
            for (double v : pct->values) total += v;
            check(near(total, 100.0), "percentages add up to 100");
        }

        AggregationResult negative;
        negative.labels = {"a", "b"};
        negative.values = {5.0, -1.0};
        check(!DerivedMetrics::toPercentages(negative).has_value(),
              "percentages refused when a value is negative");

        auto cum = DerivedMetrics::cumulative(daily.result);
        check(near(valueOf(cum, "2026-09-03"), 157000), "cumulative through 09-03 = 157000");
        check(near(valueOf(cum, "2026-09-08"), 324000), "cumulative through 09-08 = 324000");

        auto mean = Aggregator::aggregate(sales, "Product", "Sales", AggregationFunction::Mean);
        threw = false;
        try { DerivedMetrics::cumulative(mean); }
        catch (const std::invalid_argument&) { threw = true; }
        check(threw, "cumulative of averages throws");

        auto change = DerivedMetrics::percentChange(daily.result);
        check(change.size() == 7, "percent change has one fewer entry");
        check(near(valueOf(change, "2026-09-02"), -68.75), "09-02 change = -68.75%");
        check(near(valueOf(change, "2026-09-03"), 108.0), "09-03 change = +108%");

        auto sorted = DerivedMetrics::sortByValue(byProduct, true);
        check(sorted.labels[0] == "Laptop" && sorted.labels[1] == "Tablet" &&
              sorted.labels[2] == "Phone", "sorted descending: Laptop, Tablet, Phone");

        auto top2 = DerivedMetrics::topNWithOther(byProduct, 2);
        check(top2.size() == 3 && top2.labels[2] == "Other", "top 2 + Other has 3 entries");
        check(near(valueOf(top2, "Other"), 81000), "Other = Phone = 81000");

        threw = false;
        try { DerivedMetrics::topNWithOther(mean, 2); }
        catch (const std::invalid_argument&) { threw = true; }
        check(threw, "top-N of averages throws");

    } catch (const std::exception& e) {
        std::cout << "Unexpected error: " << e.what() << "\n";
        return 1;
    }

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
