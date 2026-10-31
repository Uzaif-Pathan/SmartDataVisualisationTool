#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

#include "aggregation/Aggregator.h"
#include "data/DataTable.h"

static int failures = 0;

static void check(bool ok, const std::string& what) {
    std::cout << (ok ? "[PASS] " : "[FAIL] ") << what << "\n";
    if (!ok) ++failures;
}

static double valueOf(const AggregationResult& r, const std::string& label) {
    for (size_t i = 0; i < r.labels.size(); ++i)
        if (r.labels[i] == label) return r.values[i];
    return std::nan("");
}

static bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

int main() {
    try {
        DataTable sales = DataTable::fromCsv("sample_data/sales.csv");

        auto gender = Aggregator::countByGroup(sales, "Gender");
        std::cout << gender.toText() << "\n";
        check(near(valueOf(gender, "Male"), 5), "Male count = 5");
        check(near(valueOf(gender, "Female"), 5), "Female count = 5");

        auto sum = Aggregator::aggregate(sales, "Product", "Sales", AggregationFunction::Sum);
        std::cout << sum.toText() << "\n";
        check(near(valueOf(sum, "Laptop"), 205000), "Laptop sales sum = 205000");
        check(near(valueOf(sum, "Phone"), 81000), "Phone sales sum = 81000");
        check(near(valueOf(sum, "Tablet"), 90000), "Tablet sales sum = 90000");

        auto med = Aggregator::aggregate(sales, "Product", "Sales", AggregationFunction::Median);
        check(near(valueOf(med, "Laptop"), 55000), "Laptop median = 55000");
        check(near(valueOf(med, "Phone"), 18000), "Phone median = 18000 (even count)");
        check(near(valueOf(med, "Tablet"), 30000), "Tablet median = 30000");

        auto qty = Aggregator::aggregate(sales, "Gender", "Quantity", AggregationFunction::Mean);
        std::cout << qty.toText() << "\n";
        check(near(valueOf(qty, "Male"), 1.4), "Male mean quantity = 1.4");
        check(near(valueOf(qty, "Female"), 1.75), "Female mean quantity = 1.75");
        check(qty.skippedRows == 1, "1 missing Quantity reported as skipped");

        DataTable ts = DataTable::fromCsv("sample_data/time_series.csv");
        auto daily = Aggregator::aggregate(ts, "Date", "Sales", AggregationFunction::Sum);
        std::cout << daily.toText() << "\n";
        check(daily.size() == 8, "8 distinct dates");
        check(near(valueOf(daily, "2026-09-01"), 80000), "2026-09-01 total = 80000");
        check(near(valueOf(daily, "2026-09-02"), 25000), "2026-09-02 total = 25000");
        check(near(valueOf(daily, "2026-09-03"), 52000), "2026-09-03 total = 52000");

        bool threw = false;
        try { Aggregator::countByGroup(sales, "NoSuchColumn"); }
        catch (const std::invalid_argument&) { threw = true; }
        check(threw, "unknown column name throws an error");
    } catch (const std::exception& e) {
        std::cout << "Unexpected error: " << e.what() << "\n";
        return 1;
    }

    std::cout << (failures == 0 ? "\nALL TESTS PASSED\n" : "\nSOME TESTS FAILED\n");
    return failures == 0 ? 0 : 1;
}
