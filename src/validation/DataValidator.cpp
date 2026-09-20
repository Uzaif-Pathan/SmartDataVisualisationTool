#include "DataValidator.h"
#include "MissingValueChecker.h"
#include "DuplicateChecker.h"
#include "TypeValidator.h"
#include "DateValidator.h"
#include "NumericValidator.h"
#include "EmptyColumnChecker.h"
#include "ConstantColumnChecker.h"

namespace sdv {

ValidationReport DataValidator::validate(const DataTable& table) const {
    ValidationReport report;

    report.addResults(MissingValueChecker().check(table));
    report.addResults(DuplicateChecker().check(table));
    report.addResults(EmptyColumnChecker().check(table));
    report.addResults(ConstantColumnChecker().check(table));
    report.addResults(TypeValidator().check(table));
    report.addResults(NumericValidator().check(table));
    report.addResults(DateValidator().check(table));

    return report;
}

} // namespace sdv
