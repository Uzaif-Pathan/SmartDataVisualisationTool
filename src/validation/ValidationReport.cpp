#include "ValidationReport.h"
#include <sstream>
#include <algorithm>

namespace sdv {

void ValidationReport::addResult(ValidationResult result) {
    results_.push_back(std::move(result));
}

void ValidationReport::addResults(const std::vector<ValidationResult>& results) {
    results_.insert(results_.end(), results.begin(), results.end());
}

const std::vector<ValidationResult>& ValidationReport::results() const {
    return results_;
}

std::vector<ValidationResult> ValidationReport::resultsForColumn(const std::string& columnName) const {
    std::vector<ValidationResult> out;
    for (const auto& r : results_) {
        if (r.columnName == columnName) out.push_back(r);
    }
    return out;
}

std::vector<ValidationResult> ValidationReport::resultsBySeverity(Severity severity) const {
    std::vector<ValidationResult> out;
    for (const auto& r : results_) {
        if (r.severity == severity) out.push_back(r);
    }
    return out;
}

size_t ValidationReport::errorCount() const {
    return static_cast<size_t>(std::count_if(results_.begin(), results_.end(),
        [](const ValidationResult& r) { return r.severity == Severity::Error; }));
}

size_t ValidationReport::warningCount() const {
    return static_cast<size_t>(std::count_if(results_.begin(), results_.end(),
        [](const ValidationResult& r) { return r.severity == Severity::Warning; }));
}

size_t ValidationReport::infoCount() const {
    return static_cast<size_t>(std::count_if(results_.begin(), results_.end(),
        [](const ValidationResult& r) { return r.severity == Severity::Info; }));
}

bool ValidationReport::hasErrors() const {
    return errorCount() > 0;
}

bool ValidationReport::isClean() const {
    return errorCount() == 0 && warningCount() == 0;
}

std::string ValidationReport::summary() const {
    std::ostringstream oss;
    oss << "Validation Report: " << results_.size() << " finding(s) — "
        << errorCount() << " error(s), "
        << warningCount() << " warning(s), "
        << infoCount() << " info\n";
    for (const auto& r : results_) {
        oss << "  [" << severityToString(r.severity) << "] "
            << r.checkName;
        if (!r.columnName.empty()) oss << " (column: " << r.columnName << ")";
        if (r.rowIndex >= 0) oss << " (row: " << r.rowIndex << ")";
        oss << " — " << r.message << "\n";
    }
    return oss.str();
}

} // namespace sdv
