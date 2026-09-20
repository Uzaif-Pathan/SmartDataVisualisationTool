#pragma once
// Internal helper functions shared by the checkers in this module.
// Not part of the original file list in the brief, but it lives
// entirely inside src/validation/ (Member 2's own territory), so it
// does not violate "Rule 2 — don't modify another person's module".

#include <string>

namespace sdv {
namespace validation_utils {

// True if the trimmed string can be parsed as a floating point number
// in full (no trailing garbage), e.g. "50000", "-3.14", "2e5".
bool isNumericLiteral(const std::string& text);

// True if the trimmed string matches one of the supported date/time
// formats: YYYY-MM-DD, DD/MM/YYYY, or YYYY-MM-DD HH:MM:SS.
bool isDateLiteral(const std::string& text);

std::string trim(const std::string& text);

} // namespace validation_utils
} // namespace sdv
