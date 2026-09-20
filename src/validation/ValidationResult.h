#pragma once

#include <string>

namespace sdv {

enum class Severity { Info, Warning, Error };

std::string severityToString(Severity s);

// One finding produced by a single checker (e.g. "column Sales has 1
// missing value", "row 3 is a duplicate of row 1").
struct ValidationResult {
    std::string checkName;     // e.g. "MissingValueChecker"
    std::string columnName;    // empty if the finding is row/table level
    Severity severity;
    std::string message;       // human-readable explanation
    int rowIndex = -1;         // -1 if not applicable to a single row

    ValidationResult(std::string check,
                      std::string column,
                      Severity sev,
                      std::string msg,
                      int row = -1)
        : checkName(std::move(check)),
          columnName(std::move(column)),
          severity(sev),
          message(std::move(msg)),
          rowIndex(row) {}
};

} // namespace sdv
