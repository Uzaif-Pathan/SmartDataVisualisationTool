#include <cassert>
#include <iostream>
#include <algorithm>
#include "../../src/data/DataTable.h"
#include "../../src/recommendation/PlotRecommendationEngine.h"

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

bool anyRecommendationInvolves(const std::vector<RecommendationResult>& recs,
                                const std::string& columnName) {
    return std::any_of(recs.begin(), recs.end(), [&](const RecommendationResult& r) {
        return r.xColumn == columnName || r.yColumn == columnName;
    });
}

} // namespace

int main() {
    std::cout << "=== Recommendation module tests ===\n";

    // A dataset with a clear identifier column (Customer_ID, all
    // unique across 20 rows) mixed with normal analytical columns.
    // The engine must NEVER recommend a chart involving Customer_ID.
    std::vector<std::string> names = {"Customer_ID", "Gender", "Product", "Date", "Sales"};
    std::vector<DataColumn> columns;

    std::vector<DataValue> ids;
    std::vector<DataValue> genders;
    std::vector<DataValue> products;
    std::vector<DataValue> dates;
    std::vector<DataValue> sales;
    const char* productNames[3] = {"Laptop", "Phone", "Tablet"};
    for (int i = 0; i < 20; ++i) {
        ids.push_back(DataValue(std::string("CUST-") + std::to_string(1000 + i)));
        genders.push_back(DataValue(std::string(i % 2 == 0 ? "Male" : "Female")));
        products.push_back(DataValue(std::string(productNames[i % 3])));
        dates.push_back(DataValue(std::string("2026-09-") + (i < 9 ? "0" : "") + std::to_string(i + 1)));
        sales.push_back(DataValue(1000.0 * (i + 1)));
    }
    columns.emplace_back("Customer_ID", ids);
    columns.emplace_back("Gender", genders);
    columns.emplace_back("Product", products);
    columns.emplace_back("Date", dates);
    columns.emplace_back("Sales", sales);

    DataTable table(names, columns);

    PlotRecommendationEngine engine;
    std::vector<RecommendationResult> recs = engine.recommend(table);

    check(!recs.empty(), "Engine produces at least one recommendation for a normal dataset");
    check(!anyRecommendationInvolves(recs, "Customer_ID"),
          "Engine NEVER recommends a chart involving the Customer_ID identifier column");

    bool hasDateSalesLine = std::any_of(recs.begin(), recs.end(), [](const RecommendationResult& r) {
        return r.plotType == PlotType::Line &&
               ((r.xColumn == "Date" && r.yColumn == "Sales"));
    });
    check(hasDateSalesLine, "Engine recommends a Line chart for Date x Sales (time series)");

    bool hasGenderFrequency = std::any_of(recs.begin(), recs.end(), [](const RecommendationResult& r) {
        return r.plotType == PlotType::FrequencyBar && r.xColumn == "Gender";
    });
    check(hasGenderFrequency, "Engine recommends a Frequency Bar chart for Gender alone");

    bool hasSalesHistogram = std::any_of(recs.begin(), recs.end(), [](const RecommendationResult& r) {
        return r.plotType == PlotType::Histogram && r.xColumn == "Sales";
    });
    check(hasSalesHistogram, "Engine recommends a Histogram for Sales alone");

    // Results should be sorted by descending suitability score.
    bool sortedDescending = true;
    for (size_t i = 1; i < recs.size(); ++i) {
        if (recs[i].suitabilityScore > recs[i - 1].suitabilityScore) { sortedDescending = false; break; }
    }
    check(sortedDescending, "Recommendations are sorted by descending suitability score");

    std::cout << "\n" << (failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED") << "\n";
    return failures == 0 ? 0 : 1;
}
