#include "data/DataLoader.h"
#include "statistics/StatisticsEngine.h"

#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace {

int totalTests = 0;
int passedTests = 0;
int failedTests = 0;

void reportResult(
    const std::string& testName,
    bool passed) {

    ++totalTests;

    if (passed) {
        ++passedTests;
        std::cout << "[PASS] " << testName << '\n';
    } else {
        ++failedTests;
        std::cout << "[FAIL] " << testName << '\n';
    }
}

bool nearlyEqual(
    double actual,
    double expected,
    double tolerance = 1e-9) {

    return std::fabs(actual - expected) <= tolerance;
}

} // namespace

int main() {

    std::cout
        << "========================================\n"
        << " Smart Data Visualisation Tool\n"
        << " Statistics Test Suite\n"
        << "========================================\n\n";

    const std::string datasetPath =
        "sample_data/test_sales.csv";

    sdv::DataTable table;
    std::string errorMessage;

    std::cout
        << "Loading dataset: "
        << datasetPath
        << "\n\n";

    const bool loaded =
        sdv::DataLoader::load(
            datasetPath,
            table,
            errorMessage);

    reportResult(
        "CSV dataset loads successfully",
        loaded);

    if (!loaded) {
        std::cerr
            << "\nDataset loading error: "
            << errorMessage
            << '\n';

        std::cout
            << "\n========================================\n"
            << " TEST SUMMARY\n"
            << "========================================\n"
            << "Total : " << totalTests << '\n'
            << "Passed: " << passedTests << '\n'
            << "Failed: " << failedTests << '\n';

        return 1;
    }

    reportResult(
        "Dataset has 6 columns",
        table.columnCount() == 6);

    reportResult(
        "Dataset has 5 rows",
        table.rowCount() == 5);

    // --------------------------------------------------
    // Numerical statistics: Sales
    // --------------------------------------------------

    std::cout
        << "\n----------------------------------------\n"
        << "Numerical Statistics: Sales\n"
        << "----------------------------------------\n";

    const sdv::NumericalStatisticsResult sales =
        sdv::StatisticsEngine::numerical(
            table,
            "Sales");

    std::cout << std::fixed
              << std::setprecision(6);

    std::cout
        << "Count              : " << sales.count << '\n'
        << "Missing            : " << sales.missingCount << '\n'
        << "Minimum            : " << sales.minimum << '\n'
        << "Maximum            : " << sales.maximum << '\n'
        << "Range              : " << sales.range << '\n'
        << "Mean               : " << sales.mean << '\n'
        << "Median             : " << sales.median << '\n'
        << "Mode               : " << sales.mode << '\n'
        << "Variance           : " << sales.variance << '\n'
        << "Standard Deviation : " << sales.standardDeviation << '\n'
        << "Q1                 : " << sales.q1 << '\n'
        << "Q3                 : " << sales.q3 << '\n'
        << "IQR                : " << sales.iqr << '\n'
        << "Outliers           : " << sales.outlierCount << '\n'
        << "Lower Fence        : " << sales.lowerOutlierFence << '\n'
        << "Upper Fence        : " << sales.upperOutlierFence << '\n'
        << "Skewness           : " << sales.skewness << '\n';

    reportResult(
        "Sales statistics are valid",
        sales.hasValidStatistics);

    reportResult(
        "Sales count = 5",
        sales.count == 5);

    reportResult(
        "Sales missing count = 0",
        sales.missingCount == 0);

    reportResult(
        "Sales minimum = 1200",
        nearlyEqual(sales.minimum, 1200.0));

    reportResult(
        "Sales maximum = 55000",
        nearlyEqual(sales.maximum, 55000.0));

    reportResult(
        "Sales range = 53800",
        nearlyEqual(sales.range, 53800.0));

    reportResult(
        "Sales mean = 23100",
        nearlyEqual(sales.mean, 23100.0));

    reportResult(
        "Sales median = 2500",
        nearlyEqual(sales.median, 2500.0));

    reportResult(
        "Sales mode = 55000",
        nearlyEqual(sales.mode, 55000.0));

    reportResult(
        "Sales Q1 = 1800",
        nearlyEqual(sales.q1, 1800.0));

    reportResult(
        "Sales Q3 = 55000",
        nearlyEqual(sales.q3, 55000.0));

    reportResult(
        "Sales IQR = 53200",
        nearlyEqual(sales.iqr, 53200.0));

    reportResult(
        "Sales outlier count = 0",
        sales.outlierCount == 0);

    // --------------------------------------------------
    // Numerical statistics: Quantity
    // --------------------------------------------------

    std::cout
        << "\n----------------------------------------\n"
        << "Numerical Statistics: Quantity\n"
        << "----------------------------------------\n";

    const sdv::NumericalStatisticsResult quantity =
        sdv::StatisticsEngine::numerical(
            table,
            "Quantity");

    std::cout
        << "Count              : " << quantity.count << '\n'
        << "Missing            : " << quantity.missingCount << '\n'
        << "Minimum            : " << quantity.minimum << '\n'
        << "Maximum            : " << quantity.maximum << '\n'
        << "Range              : " << quantity.range << '\n'
        << "Mean               : " << quantity.mean << '\n'
        << "Median             : " << quantity.median << '\n'
        << "Mode               : " << quantity.mode << '\n'
        << "Variance           : " << quantity.variance << '\n'
        << "Standard Deviation : " << quantity.standardDeviation << '\n'
        << "Q1                 : " << quantity.q1 << '\n'
        << "Q3                 : " << quantity.q3 << '\n'
        << "IQR                : " << quantity.iqr << '\n'
        << "Outliers           : " << quantity.outlierCount << '\n';

    reportResult(
        "Quantity statistics are valid",
        quantity.hasValidStatistics);

    reportResult(
        "Quantity count = 5",
        quantity.count == 5);

    reportResult(
        "Quantity missing count = 0",
        quantity.missingCount == 0);

    reportResult(
        "Quantity minimum = 1",
        nearlyEqual(quantity.minimum, 1.0));

    reportResult(
        "Quantity maximum = 3",
        nearlyEqual(quantity.maximum, 3.0));

    reportResult(
        "Quantity mean = 1.6",
        nearlyEqual(quantity.mean, 1.6));

    reportResult(
        "Quantity median = 1",
        nearlyEqual(quantity.median, 1.0));

    reportResult(
        "Quantity mode = 1",
        nearlyEqual(quantity.mode, 1.0));

    reportResult(
        "Quantity Q1 = 1",
        nearlyEqual(quantity.q1, 1.0));

    reportResult(
        "Quantity Q3 = 2",
        nearlyEqual(quantity.q3, 2.0));

    reportResult(
        "Quantity IQR = 1",
        nearlyEqual(quantity.iqr, 1.0));

    reportResult(
        "Quantity outlier count = 0",
        quantity.outlierCount == 0);

    // --------------------------------------------------
    // Categorical statistics: Product
    // --------------------------------------------------

    std::cout
        << "\n----------------------------------------\n"
        << "Categorical Statistics: Product\n"
        << "----------------------------------------\n";

    const sdv::CategoricalStatisticsResult product =
        sdv::StatisticsEngine::categorical(
            table,
            "Product");

    std::cout
        << "Count              : " << product.count << '\n'
        << "Missing            : " << product.missingCount << '\n'
        << "Unique categories  : " << product.uniqueCount << '\n'
        << "Most frequent      : "
        << product.mostFrequentValue
        << " (" << product.mostFrequentCount << ")\n"
        << "Least frequent     : "
        << product.leastFrequentValue
        << " (" << product.leastFrequentCount << ")\n";

    std::cout << "\nCategory frequencies:\n";

    for (const auto& frequency :
         product.frequencies) {

        std::cout
            << "  "
            << frequency.value
            << " -> "
            << frequency.count
            << " ("
            << frequency.percentage
            << "%)\n";
    }

    reportResult(
        "Product categorical statistics are valid",
        product.hasValidStatistics);

    reportResult(
        "Product count = 5",
        product.count == 5);

    reportResult(
        "Product missing count = 0",
        product.missingCount == 0);

    reportResult(
        "Product unique count = 3",
        product.uniqueCount == 3);

    reportResult(
        "Laptop frequency = 2",
        product.frequencies.size() >= 1 &&
        product.frequencies[0].value == "Laptop" &&
        product.frequencies[0].count == 2);

    reportResult(
        "Mouse frequency = 2",
        product.frequencies.size() >= 2 &&
        product.frequencies[1].value == "Mouse" &&
        product.frequencies[1].count == 2);

    reportResult(
        "Keyboard frequency = 1",
        product.frequencies.size() >= 3 &&
        product.frequencies[2].value == "Keyboard" &&
        product.frequencies[2].count == 1);

    reportResult(
        "Most frequent product = Laptop",
        product.mostFrequentValue == "Laptop");

    // --------------------------------------------------
    // Pearson correlation: Sales vs Quantity
    // --------------------------------------------------

    std::cout
        << "\n----------------------------------------\n"
        << "Pearson Correlation: Sales vs Quantity\n"
        << "----------------------------------------\n";

    const sdv::CorrelationResult correlation =
        sdv::StatisticsEngine::correlation(
            table,
            "Sales",
            "Quantity");

    std::cout
        << "First column       : "
        << correlation.firstColumn << '\n'
        << "Second column      : "
        << correlation.secondColumn << '\n'
        << "Paired observations: "
        << correlation.pairedCount << '\n'
        << "Covariance         : "
        << correlation.covariance << '\n'
        << "Pearson correlation: "
        << correlation.pearsonCorrelation << '\n';

    /*
 * Expected Pearson correlation is approximately:
 *
 * -0.619013...
 *
 * for:
 *
 * Sales    = 55000, 1200, 2500, 55000, 1800
 * Quantity = 1,     2,    1,    1,     3
 */
    reportResult(
        "Correlation result is valid",
        correlation.valid);

    reportResult(
        "Correlation paired count = 5",
        correlation.pairedCount == 5);

reportResult(
    "Pearson correlation is approximately -0.619014",
    nearlyEqual(
        correlation.pearsonCorrelation,
        -0.619014,
        1e-6));

    // --------------------------------------------------
    // Correlation matrix
    // --------------------------------------------------

    std::cout
        << "\n----------------------------------------\n"
        << "Correlation Matrix Test\n"
        << "----------------------------------------\n";

    const std::vector<std::string> numericColumns = {
        "Sales",
        "Quantity"
    };

    const sdv::CorrelationMatrix matrix =
        sdv::StatisticsEngine::correlationMatrix(
            table,
            numericColumns);

    reportResult(
        "Correlation matrix has 2 columns",
        matrix.columnNames.size() == 2);

    reportResult(
        "Correlation matrix has 2 rows",
        matrix.values.size() == 2);

    bool diagonalValid =
        matrix.values.size() == 2 &&
        matrix.values[0].size() == 2 &&
        matrix.values[1].size() == 2 &&
        nearlyEqual(matrix.values[0][0], 1.0) &&
        nearlyEqual(matrix.values[1][1], 1.0);

    reportResult(
        "Correlation matrix diagonal = 1",
        diagonalValid);

    bool matrixSymmetric =
        matrix.values.size() == 2 &&
        matrix.values[0].size() == 2 &&
        matrix.values[1].size() == 2 &&
        nearlyEqual(
            matrix.values[0][1],
            matrix.values[1][0],
            1e-9);

    reportResult(
        "Correlation matrix is symmetric",
        matrixSymmetric);

    reportResult(
        "Correlation matrix Sales/Quantity matches Pearson result",
        matrix.values.size() == 2 &&
        matrix.values[0].size() == 2 &&
        nearlyEqual(
            matrix.values[0][1],
            correlation.pearsonCorrelation,
            1e-9));

    // --------------------------------------------------
    // Final summary
    // --------------------------------------------------

    std::cout
        << "\n========================================\n"
        << " TEST SUMMARY\n"
        << "========================================\n"
        << "Total : " << totalTests << '\n'
        << "Passed: " << passedTests << '\n'
        << "Failed: " << failedTests << '\n';

    if (failedTests == 0) {
        std::cout
            << "\nRESULT: ALL TESTS PASSED\n";

        return 0;
    }

    std::cout
        << "\nRESULT: TESTS FAILED\n";

    return 1;
}
