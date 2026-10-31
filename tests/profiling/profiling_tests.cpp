#include <cassert>
#include <iostream>
#include "../../src/data/DataTable.h"
#include "../../src/profiling/ColumnProfiler.h"

using namespace sdv;

namespace {

int failures = 0;

void check(bool condition, const std::string& description) {
    if (condition) {
        std::cout << "  [PASS] " << description << "\n";
    } else {
        std::cout << "  [FAIL] " << description << "\n";
        ++failures;
    }
}

} // namespace

int main() {
    std::cout << "=== Profiling module tests ===\n";

    // 1000-row style identifier test: 998 unique out of 1000 should
    // score as a likely identifier, exactly as the brief's example
    // ("Customer_ID: 1000 rows, 998 unique -> identifier").
    {
        std::vector<DataValue> idValues;
        for (int i = 0; i < 998; ++i) idValues.push_back(DataValue(std::string("ID" + std::to_string(i))));
        idValues.push_back(DataValue(std::string("ID0")));   // 1 repeat
        idValues.push_back(DataValue(std::string("ID1")));   // 1 repeat -> 998 unique / 1000 rows
        DataColumn idColumn("Customer_ID", idValues);

        ColumnProfiler profiler;
        ColumnProfile profile = profiler.profileColumn(idColumn);
        check(profile.isLikelyIdentifier, "Customer_ID (998/1000 unique) is detected as Identifier");
        check(profile.type == DetectedType::Identifier, "Customer_ID type is Identifier, not Numeric/Unknown");
    }

    // Gender-style low-cardinality categorical test from the brief:
    // 1000 rows, 2 unique -> categorical, not identifier.
    {
        std::vector<DataValue> genderValues;
        for (int i = 0; i < 500; ++i) genderValues.push_back(DataValue(std::string("Male")));
        for (int i = 0; i < 500; ++i) genderValues.push_back(DataValue(std::string("Female")));
        DataColumn genderColumn("Gender", genderValues);

        ColumnProfiler profiler;
        ColumnProfile profile = profiler.profileColumn(genderColumn);
        check(!profile.isLikelyIdentifier, "Gender (2/1000 unique) is NOT detected as Identifier");
        check(profile.type == DetectedType::Categorical, "Gender is detected as Categorical");
        check(profile.topCategories.size() == 2, "Gender frequency table has exactly 2 categories");
    }

    // Date detection across supported formats.
    {
        DataColumn dateColumn("Order_Date", std::vector<DataValue>{
            DataValue(std::string("2026-09-01")),
            DataValue(std::string("2026-09-02 14:30:00")),
            DataValue(std::string("2026-09-03")),
        });
        ColumnProfiler profiler;
        ColumnProfile profile = profiler.profileColumn(dateColumn);
        check(profile.type == DetectedType::DateTime, "ISO date/time column detected as DateTime");
    }

    // Numeric detection.
    {
        DataColumn numColumn("Quantity", std::vector<DataValue>{
            DataValue(10.0), DataValue(20.0), DataValue(30.0),
        });
        ColumnProfiler profiler;
        ColumnProfile profile = profiler.profileColumn(numColumn);
        check(profile.type == DetectedType::Numeric, "Pure-numeric column detected as Numeric");
    }

    // Missingness percentage sanity check.
    {
        DataColumn colWithMissing("Sales", std::vector<DataValue>{
            DataValue(100.0), DataValue(), DataValue(200.0), DataValue(),
        });
        ColumnProfiler profiler;
        ColumnProfile profile = profiler.profileColumn(colWithMissing);
        check(profile.missingCount == 2, "Missing count correctly counted as 2");
        check(profile.missingPercent > 49.0 && profile.missingPercent < 51.0,
              "Missing percent is ~50%");
    }

    std::cout << "\n" << (failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << "\n";
    return failures == 0 ? 0 : 1;
}
