#pragma once

#include <string>
#include <vector>
#include "ValidationResult.h"

namespace sdv {

// Collects every ValidationResult produced by the individual checkers
// and offers convenience queries over them. This is what DataValidator
// hands back to the caller (and, downstream, to ColumnProfiler / the
// recommendation engine, which may choose to ignore columns with
// ERROR-level findings).
class ValidationReport {
public:
    void addResult(ValidationResult result);
    void addResults(const std::vector<ValidationResult>& results);

    const std::vector<ValidationResult>& results() const;

    std::vector<ValidationResult> resultsForColumn(const std::string& columnName) const;
    std::vector<ValidationResult> resultsBySeverity(Severity severity) const;

    size_t errorCount() const;
    size_t warningCount() const;
    size_t infoCount() const;
    bool hasErrors() const;
    bool isClean() const; // no warnings or errors at all

    // Human-readable multi-line summary, useful for logging/CLI output.
    std::string summary() const;

private:
    std::vector<ValidationResult> results_;
};

} // namespace sdv
