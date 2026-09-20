#ifndef SMARTDATAVISUALISATION_CSVPARSER_H
#define SMARTDATAVISUALISATION_CSVPARSER_H

#include <string>
#include <vector>

namespace sdv {

class CSVParser {
public:
    static std::vector<std::string> parseLine(
        const std::string& line);

    static std::string escapeField(
        const std::string& value);

    static std::string buildLine(
        const std::vector<std::string>& fields);
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_CSVPARSER_H
