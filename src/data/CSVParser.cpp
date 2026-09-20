#include "CSVParser.h"

namespace sdv {

std::vector<std::string> CSVParser::parseLine(
    const std::string& line) {

    std::vector<std::string> fields;
    std::string current;

    bool insideQuotes = false;

    for (std::size_t i = 0; i < line.size(); ++i) {
        const char character = line[i];

        if (character == '"') {
            if (insideQuotes &&
                i + 1 < line.size() &&
                line[i + 1] == '"') {

                current += '"';
                ++i;
            }
            else {
                insideQuotes = !insideQuotes;
            }
        }
        else if (character == ',' && !insideQuotes) {
            fields.push_back(current);
            current.clear();
        }
        else {
            current += character;
        }
    }

    fields.push_back(current);

    return fields;
}

std::string CSVParser::escapeField(
    const std::string& value) {

    bool needsQuotes = false;

    for (char character : value) {
        if (character == ',' ||
            character == '"' ||
            character == '\n' ||
            character == '\r') {

            needsQuotes = true;
            break;
        }
    }

    if (!needsQuotes) {
        return value;
    }

    std::string escaped = "\"";

    for (char character : value) {
        if (character == '"') {
            escaped += "\"\"";
        }
        else {
            escaped += character;
        }
    }

    escaped += '"';

    return escaped;
}

std::string CSVParser::buildLine(
    const std::vector<std::string>& fields) {

    std::string line;

    for (std::size_t i = 0; i < fields.size(); ++i) {
        line += escapeField(fields[i]);

        if (i + 1 < fields.size()) {
            line += ',';
        }
    }

    return line;
}

} // namespace sdv
