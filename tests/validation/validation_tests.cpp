// Lightweight assert-based unit tests for src/validation/.
// No external test framework dependency — keeps the build simple for
// a college project. Run the binary; it prints PASS/FAIL per case and
// returns non-zero on any failure (suitable for CI).
#include <cassert>
#include <iostream>
#include "../../src/data/DataTable.h"
#include "../../src/validation/DataValidator.h"

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

DataTable tableWithIssues() {
    std::vector<std::string> names = {"Name", "Sales", "Country", "Unused"};
    std::vector<DataColumn> columns;

    columns.emplace_back("Name", std::vector<DataValue>{
        DataValue(std::string("Rahul")), DataValue(std::string("Priya")),
        DataValue(std::string("Rahul")), DataValue(std::string("Amit")),
    });
    columns.emplace_back("Sales", std::vector<DataValue>{
        DataValue(50000.0), DataValue(), DataValue(std::string("abc")), DataValue(25000.0),
    });
    columns.emplace_back("Country", std::vector<DataValue>{
        DataValue(std::string("India")), DataValue(std::string("India")),
        DataValue(std::string("India")), DataValue(std::string("India")),
    });
    columns.emplace_back("Unused", std::vector<DataValue>{
        DataValue(), DataValue(), DataValue(), DataValue(),
    });
    // Row 2 duplicates row 0 (Rahul, "abc" != 50000 though -> not a
    // full duplicate). Adjust Sales so row 2 truly repeats row 0.
    return DataTable(names, columns);
}

} // namespace

int main() {
    std::cout << "=== Validation module tests ===\n";
    DataTable table = tableWithIssues();
    DataValidator validator;
    ValidationReport report = validator.validate(table);

    check(!report.resultsForColumn("Sales").empty(),
          "MissingValueChecker/NumericValidator flag issues in Sales column");
    check(!report.resultsForColumn("Unused").empty(),
          "EmptyColumnChecker flags fully-empty Unused column");
    check(!report.resultsForColumn("Country").empty(),
          "ConstantColumnChecker flags single-value Country column");

    bool foundInvalidNumeric = false;
    for (const auto& r : report.results()) {
        if (r.checkName == "NumericValidator" && r.columnName == "Sales") foundInvalidNumeric = true;
    }
    check(foundInvalidNumeric, "NumericValidator specifically catches \"abc\" in Sales");

    check(report.hasErrors(), "Report correctly flags at least one ERROR severity finding");

    // Clean table -> no findings.
    std::vector<std::string> cleanNames = {"Product", "Sales"};
    std::vector<DataColumn> cleanColumns;
    cleanColumns.emplace_back("Product", std::vector<DataValue>{
        DataValue(std::string("Laptop")), DataValue(std::string("Phone")),
        DataValue(std::string("Tablet")),
    });
    cleanColumns.emplace_back("Sales", std::vector<DataValue>{
        DataValue(1000.0), DataValue(2000.0), DataValue(3000.0),
    });
    DataTable cleanTable(cleanNames, cleanColumns);
    ValidationReport cleanReport = validator.validate(cleanTable);
    check(cleanReport.isClean(), "A clean, fully-populated table produces no warnings/errors");

    std::cout << "\n" << (failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << "\n";
    return failures == 0 ? 0 : 1;
}
