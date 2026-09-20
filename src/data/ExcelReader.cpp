#include "ExcelReader.h"

#include <archive.h>
#include <archive_entry.h>
#include <tinyxml2.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace sdv {

namespace {

using namespace tinyxml2;

struct ExcelStyles {
    std::unordered_map<unsigned int, std::string> customFormats;
    std::vector<unsigned int> styleNumberFormats;
};

struct CellData {
    std::size_t columnIndex = 0;
    DataValue value;
};

using ParsedRow = std::vector<CellData>;

std::string localName(const char* name) {
    if (name == nullptr) {
        return {};
    }

    const std::string fullName(name);
    const std::size_t position = fullName.find(':');

    if (position == std::string::npos) {
        return fullName;
    }

    return fullName.substr(position + 1);
}

XMLElement* firstChildByLocalName(
    XMLNode* parent,
    const std::string& wantedName) {

    if (parent == nullptr) {
        return nullptr;
    }

    for (XMLElement* element = parent->FirstChildElement();
         element != nullptr;
         element = element->NextSiblingElement()) {

        if (localName(element->Name()) == wantedName) {
            return element;
        }
    }

    return nullptr;
}

void collectElementsByLocalName(
    XMLNode* parent,
    const std::string& wantedName,
    std::vector<XMLElement*>& output) {

    if (parent == nullptr) {
        return;
    }

    for (XMLElement* element = parent->FirstChildElement();
         element != nullptr;
         element = element->NextSiblingElement()) {

        if (localName(element->Name()) == wantedName) {
            output.push_back(element);
        }

        collectElementsByLocalName(
            element,
            wantedName,
            output);
    }
}

std::string collectTextElements(XMLNode* parent) {
    std::vector<XMLElement*> textElements;

    collectElementsByLocalName(
        parent,
        "t",
        textElements);

    std::string result;

    for (XMLElement* element : textElements) {
        const char* text = element->GetText();

        if (text != nullptr) {
            result += text;
        }
    }

    return result;
}

bool readArchiveEntry(
    const std::string& archivePath,
    const std::string& entryPath,
    std::string& output,
    std::string& errorMessage,
    bool required = true) {

    output.clear();

    struct archive* archiveHandle =
        archive_read_new();

    if (archiveHandle == nullptr) {
        errorMessage =
            "Unable to create archive reader.";

        return false;
    }

    archive_read_support_filter_all(archiveHandle);
    archive_read_support_format_zip(archiveHandle);

    const int openResult =
        archive_read_open_filename(
            archiveHandle,
            archivePath.c_str(),
            10240);

    if (openResult != ARCHIVE_OK) {
        errorMessage =
            "Unable to open Excel file as an XLSX archive: ";

        const char* archiveError =
            archive_error_string(archiveHandle);

        if (archiveError != nullptr) {
            errorMessage += archiveError;
        }

        archive_read_free(archiveHandle);
        return false;
    }

    struct archive_entry* entry = nullptr;
    bool found = false;

    while (archive_read_next_header(
               archiveHandle,
               &entry) == ARCHIVE_OK) {

        const char* currentPath =
            archive_entry_pathname(entry);

        if (currentPath != nullptr &&
            entryPath == currentPath) {

            found = true;

            const void* block = nullptr;
            std::size_t blockSize = 0;
            la_int64_t offset = 0;

            while (true) {
                const int result =
                    archive_read_data_block(
                        archiveHandle,
                        &block,
                        &blockSize,
                        &offset);

                if (result == ARCHIVE_EOF) {
                    break;
                }

                if (result != ARCHIVE_OK) {
                    errorMessage =
                        "Failed while reading XLSX component: " +
                        entryPath;

                    archive_read_close(archiveHandle);
                    archive_read_free(archiveHandle);

                    return false;
                }

                output.append(
                    static_cast<const char*>(block),
                    blockSize);
            }

            break;
        }

        archive_read_data_skip(archiveHandle);
    }

    archive_read_close(archiveHandle);
    archive_read_free(archiveHandle);

    if (!found && required) {
        errorMessage =
            "Required XLSX component was not found: " +
            entryPath;

        return false;
    }

    return found;
}

bool parseDouble(
    const std::string& text,
    double& value) {

    if (text.empty()) {
        return false;
    }

    char* endPointer = nullptr;

    value = std::strtod(
        text.c_str(),
        &endPointer);

    if (endPointer == text.c_str()) {
        return false;
    }

    while (*endPointer != '\0') {
        if (!std::isspace(
                static_cast<unsigned char>(*endPointer))) {
            return false;
        }

        ++endPointer;
    }

    return true;
}

bool parseUnsignedInteger(
    const std::string& text,
    unsigned int& value) {

    if (text.empty()) {
        return false;
    }

    char* endPointer = nullptr;

    const unsigned long parsed =
        std::strtoul(
            text.c_str(),
            &endPointer,
            10);

    if (endPointer == text.c_str() ||
        *endPointer != '\0') {

        return false;
    }

    value =
        static_cast<unsigned int>(parsed);

    return true;
}

std::size_t columnIndexFromReference(
    const std::string& reference) {

    std::size_t result = 0;
    bool foundLetter = false;

    for (char character : reference) {
        if (!std::isalpha(
                static_cast<unsigned char>(character))) {
            break;
        }

        foundLetter = true;

        const char upper =
            static_cast<char>(
                std::toupper(
                    static_cast<unsigned char>(character)));

        result =
            result * 26 +
            static_cast<std::size_t>(
                upper - 'A' + 1);
    }

    if (!foundLetter || result == 0) {
        return 0;
    }

    return result - 1;
}

std::string normalizeRelationshipTarget(
    const std::string& target) {

    if (target.empty()) {
        return {};
    }

    if (target.front() == '/') {
        return target.substr(1);
    }

    std::filesystem::path fullPath =
        std::filesystem::path("xl") /
        std::filesystem::path(target);

    return fullPath
        .lexically_normal()
        .generic_string();
}

bool loadSharedStrings(
    const std::string& workbookPath,
    std::vector<std::string>& sharedStrings,
    std::string& errorMessage) {

    std::string xml;

    const bool exists =
        readArchiveEntry(
            workbookPath,
            "xl/sharedStrings.xml",
            xml,
            errorMessage,
            false);

    if (!exists) {
        errorMessage.clear();
        return true;
    }

    XMLDocument document;

    if (document.Parse(
            xml.data(),
            xml.size()) != XML_SUCCESS) {

        errorMessage =
            "Unable to parse xl/sharedStrings.xml.";

        return false;
    }

    XMLElement* root =
        document.RootElement();

    if (root == nullptr) {
        return true;
    }

    for (XMLElement* element = root->FirstChildElement();
         element != nullptr;
         element = element->NextSiblingElement()) {

        if (localName(element->Name()) != "si") {
            continue;
        }

        sharedStrings.push_back(
            collectTextElements(element));
    }

    return true;
}

bool getFirstWorksheetPath(
    const std::string& workbookPath,
    std::string& worksheetPath,
    bool& uses1904DateSystem,
    std::string& errorMessage) {

    uses1904DateSystem = false;

    std::string workbookXml;

    if (!readArchiveEntry(
            workbookPath,
            "xl/workbook.xml",
            workbookXml,
            errorMessage)) {

        return false;
    }

    XMLDocument workbookDocument;

    if (workbookDocument.Parse(
            workbookXml.data(),
            workbookXml.size()) != XML_SUCCESS) {

        errorMessage =
            "Unable to parse xl/workbook.xml.";

        return false;
    }

    XMLElement* workbookRoot =
        workbookDocument.RootElement();

    if (workbookRoot == nullptr) {
        errorMessage =
            "Invalid Excel workbook XML.";

        return false;
    }

    for (XMLElement* element =
             workbookRoot->FirstChildElement();
         element != nullptr;
         element = element->NextSiblingElement()) {

        if (localName(element->Name()) ==
            "workbookPr") {

            const char* date1904 =
                element->Attribute("date1904");

            if (date1904 != nullptr) {
                const std::string value(date1904);

                uses1904DateSystem =
                    value == "1" ||
                    value == "true";
            }
        }
    }

    XMLElement* sheetsElement =
        firstChildByLocalName(
            workbookRoot,
            "sheets");

    if (sheetsElement == nullptr) {
        errorMessage =
            "Excel workbook contains no worksheets.";

        return false;
    }

    XMLElement* firstSheet = nullptr;

    for (XMLElement* element =
             sheetsElement->FirstChildElement();
         element != nullptr;
         element = element->NextSiblingElement()) {

        if (localName(element->Name()) == "sheet") {
            firstSheet = element;
            break;
        }
    }

    if (firstSheet == nullptr) {
        errorMessage =
            "Excel workbook contains no worksheets.";

        return false;
    }

    const char* relationshipId =
        firstSheet->Attribute("r:id");

    if (relationshipId == nullptr) {
        for (const XMLAttribute* attribute =
                 firstSheet->FirstAttribute();
             attribute != nullptr;
             attribute = attribute->Next()) {

            if (localName(attribute->Name()) == "id") {
                relationshipId =
                    attribute->Value();

                break;
            }
        }
    }

    if (relationshipId == nullptr) {
        errorMessage =
            "Unable to identify the first worksheet.";

        return false;
    }

    std::string relationshipsXml;

    if (!readArchiveEntry(
            workbookPath,
            "xl/_rels/workbook.xml.rels",
            relationshipsXml,
            errorMessage)) {

        return false;
    }

    XMLDocument relationshipsDocument;

    if (relationshipsDocument.Parse(
            relationshipsXml.data(),
            relationshipsXml.size()) != XML_SUCCESS) {

        errorMessage =
            "Unable to parse workbook relationships.";

        return false;
    }

    XMLElement* relationshipsRoot =
        relationshipsDocument.RootElement();

    if (relationshipsRoot == nullptr) {
        errorMessage =
            "Invalid workbook relationships XML.";

        return false;
    }

    for (XMLElement* relationship =
             relationshipsRoot->FirstChildElement();
         relationship != nullptr;
         relationship =
             relationship->NextSiblingElement()) {

        if (localName(relationship->Name()) !=
            "Relationship") {

            continue;
        }

        const char* id =
            relationship->Attribute("Id");

        const char* target =
            relationship->Attribute("Target");

        if (id != nullptr &&
            target != nullptr &&
            std::string(id) == relationshipId) {

            worksheetPath =
                normalizeRelationshipTarget(target);

            return true;
        }
    }

    errorMessage =
        "Unable to resolve the first worksheet file.";

    return false;
}

bool loadStyles(
    const std::string& workbookPath,
    ExcelStyles& styles,
    std::string& errorMessage) {

    std::string stylesXml;

    const bool exists =
        readArchiveEntry(
            workbookPath,
            "xl/styles.xml",
            stylesXml,
            errorMessage,
            false);

    if (!exists) {
        errorMessage.clear();
        return true;
    }

    XMLDocument document;

    if (document.Parse(
            stylesXml.data(),
            stylesXml.size()) != XML_SUCCESS) {

        errorMessage =
            "Unable to parse xl/styles.xml.";

        return false;
    }

    XMLElement* root =
        document.RootElement();

    if (root == nullptr) {
        return true;
    }

    XMLElement* numberFormats =
        firstChildByLocalName(
            root,
            "numFmts");

    if (numberFormats != nullptr) {
        for (XMLElement* format =
                 numberFormats->FirstChildElement();
             format != nullptr;
             format = format->NextSiblingElement()) {

            if (localName(format->Name()) != "numFmt") {
                continue;
            }

            unsigned int id = 0;

            if (format->QueryUnsignedAttribute(
                    "numFmtId",
                    &id) != XML_SUCCESS) {

                continue;
            }

            const char* formatCode =
                format->Attribute("formatCode");

            if (formatCode != nullptr) {
                styles.customFormats[id] =
                    formatCode;
            }
        }
    }

    XMLElement* cellFormats =
        firstChildByLocalName(
            root,
            "cellXfs");

    if (cellFormats != nullptr) {
        for (XMLElement* style =
                 cellFormats->FirstChildElement();
             style != nullptr;
             style = style->NextSiblingElement()) {

            if (localName(style->Name()) != "xf") {
                continue;
            }

            unsigned int numberFormatId = 0;

            style->QueryUnsignedAttribute(
                "numFmtId",
                &numberFormatId);

            styles.styleNumberFormats.push_back(
                numberFormatId);
        }
    }

    return true;
}

std::string cleanedFormatCode(
    std::string format) {

    std::string output;
    bool insideQuotes = false;

    for (std::size_t i = 0;
         i < format.size();
         ++i) {

        const char character = format[i];

        if (character == '"') {
            insideQuotes = !insideQuotes;
            continue;
        }

        if (insideQuotes) {
            continue;
        }

        if (character == '\\' &&
            i + 1 < format.size()) {

            ++i;
            continue;
        }

        if (character == '_' &&
            i + 1 < format.size()) {

            ++i;
            continue;
        }

        if (character == '*' &&
            i + 1 < format.size()) {

            ++i;
            continue;
        }

        output +=
            static_cast<char>(
                std::tolower(
                    static_cast<unsigned char>(
                        character)));
    }

    return output;
}

bool builtInDateFormat(
    unsigned int numberFormatId) {

    if (numberFormatId >= 14 &&
        numberFormatId <= 22) {

        return true;
    }

    if (numberFormatId >= 27 &&
        numberFormatId <= 36) {

        return true;
    }

    if (numberFormatId >= 45 &&
        numberFormatId <= 47) {

        return true;
    }

    if (numberFormatId >= 50 &&
        numberFormatId <= 58) {

        return true;
    }

    return false;
}

bool customDateFormat(
    const std::string& formatCode) {

    const std::string format =
        cleanedFormatCode(formatCode);

    bool hasDateOrTimeToken = false;

    for (char character : format) {
        if (character == 'y' ||
            character == 'd' ||
            character == 'h' ||
            character == 's') {

            hasDateOrTimeToken = true;
            break;
        }
    }

    if (!hasDateOrTimeToken) {
        if (format.find("m/") != std::string::npos ||
            format.find("/m") != std::string::npos ||
            format.find("m-") != std::string::npos ||
            format.find("-m") != std::string::npos) {

            hasDateOrTimeToken = true;
        }
    }

    return hasDateOrTimeToken;
}

bool isDateStyle(
    unsigned int styleIndex,
    const ExcelStyles& styles) {

    if (styleIndex >=
        styles.styleNumberFormats.size()) {

        return false;
    }

    const unsigned int numberFormatId =
        styles.styleNumberFormats[styleIndex];

    if (builtInDateFormat(numberFormatId)) {
        return true;
    }

    const auto iterator =
        styles.customFormats.find(
            numberFormatId);

    if (iterator == styles.customFormats.end()) {
        return false;
    }

    return customDateFormat(
        iterator->second);
}

long long daysFromCivil(
    int year,
    unsigned int month,
    unsigned int day) {

    year -= month <= 2;

    const int era =
        (year >= 0 ? year : year - 399) / 400;

    const unsigned int yearOfEra =
        static_cast<unsigned int>(
            year - era * 400);

    const unsigned int dayOfYear =
        (153 *
             (month +
              (month > 2 ? -3 : 9)) +
         2) /
            5 +
        day - 1;

    const unsigned int dayOfEra =
        yearOfEra * 365 +
        yearOfEra / 4 -
        yearOfEra / 100 +
        dayOfYear;

    return static_cast<long long>(era) *
               146097LL +
           static_cast<long long>(dayOfEra) -
           719468LL;
}

void civilFromDays(
    long long days,
    int& year,
    unsigned int& month,
    unsigned int& day) {

    days += 719468LL;

    const long long era =
        (days >= 0 ? days : days - 146096LL) /
        146097LL;

    const unsigned int dayOfEra =
        static_cast<unsigned int>(
            days - era * 146097LL);

    const unsigned int yearOfEra =
        (dayOfEra -
         dayOfEra / 1460 +
         dayOfEra / 36524 -
         dayOfEra / 146096) /
        365;

    year =
        static_cast<int>(yearOfEra) +
        static_cast<int>(era * 400);

    const unsigned int dayOfYear =
        dayOfEra -
        (365 * yearOfEra +
         yearOfEra / 4 -
         yearOfEra / 100);

    const unsigned int monthPrime =
        (5 * dayOfYear + 2) / 153;

    day =
        dayOfYear -
        (153 * monthPrime + 2) / 5 +
        1;

    month =
        monthPrime +
        (monthPrime < 10 ? 3 : -9);

    year += month <= 2;
}

std::string excelSerialToDateTime(
    double serial,
    bool uses1904DateSystem) {

    double wholePart = 0.0;

    double fraction =
        std::modf(
            serial,
            &wholePart);

    if (fraction < 0.0) {
        fraction += 1.0;
        wholePart -= 1.0;
    }

    long long serialDays =
        static_cast<long long>(wholePart);

    long long baseDays = 0;

    if (uses1904DateSystem) {
        baseDays =
            daysFromCivil(
                1904,
                1,
                1);
    }
    else {
        baseDays =
            daysFromCivil(
                1899,
                12,
                31);

        if (serialDays >= 60) {
            --serialDays;
        }
    }

    const long long absoluteDays =
        baseDays + serialDays;

    int year = 0;
    unsigned int month = 0;
    unsigned int day = 0;

    civilFromDays(
        absoluteDays,
        year,
        month,
        day);

    long long seconds =
        static_cast<long long>(
            std::round(
                fraction * 86400.0));

    if (seconds >= 86400) {
        seconds -= 86400;

        civilFromDays(
            absoluteDays + 1,
            year,
            month,
            day);
    }

    const int hour =
        static_cast<int>(
            seconds / 3600);

    seconds %= 3600;

    const int minute =
        static_cast<int>(
            seconds / 60);

    const int second =
        static_cast<int>(
            seconds % 60);

    std::ostringstream stream;

    stream
        << std::setfill('0')
        << std::setw(4)
        << year
        << '-'
        << std::setw(2)
        << month
        << '-'
        << std::setw(2)
        << day;

    if (hour != 0 ||
        minute != 0 ||
        second != 0) {

        stream
            << ' '
            << std::setw(2)
            << hour
            << ':'
            << std::setw(2)
            << minute
            << ':'
            << std::setw(2)
            << second;
    }

    return stream.str();
}

DataValue parseCellValue(
    XMLElement* cell,
    const std::vector<std::string>& sharedStrings,
    const ExcelStyles& styles,
    bool uses1904DateSystem) {

    if (cell == nullptr) {
        return DataValue();
    }

    const char* typeAttribute =
        cell->Attribute("t");

    const std::string type =
        typeAttribute != nullptr
            ? typeAttribute
            : "";

    unsigned int styleIndex = 0;

    const char* styleAttribute =
        cell->Attribute("s");

    if (styleAttribute != nullptr) {
        parseUnsignedInteger(
            styleAttribute,
            styleIndex);
    }

    if (type == "inlineStr") {
        XMLElement* inlineString =
            firstChildByLocalName(
                cell,
                "is");

        if (inlineString == nullptr) {
            return DataValue();
        }

        return DataValue(
            collectTextElements(
                inlineString));
    }

    XMLElement* valueElement =
        firstChildByLocalName(
            cell,
            "v");

    if (valueElement == nullptr ||
        valueElement->GetText() == nullptr) {

        return DataValue();
    }

    const std::string rawValue =
        valueElement->GetText();

    if (type == "s") {
        unsigned int index = 0;

        if (!parseUnsignedInteger(
                rawValue,
                index)) {

            return DataValue();
        }

        if (index >= sharedStrings.size()) {
            return DataValue();
        }

        return DataValue(
            sharedStrings[index]);
    }

    if (type == "b") {
        return DataValue(
            rawValue == "1" ||
            rawValue == "true");
    }

    if (type == "str" ||
        type == "e") {

        return DataValue(rawValue);
    }

    double numericValue = 0.0;

    if (parseDouble(
            rawValue,
            numericValue)) {

        if (isDateStyle(
                styleIndex,
                styles)) {

            return DataValue(
                excelSerialToDateTime(
                    numericValue,
                    uses1904DateSystem));
        }

        return DataValue(numericValue);
    }

    return DataValue(rawValue);
}

std::string makeUniqueColumnName(
    const std::string& requestedName,
    std::set<std::string>& usedNames,
    std::size_t columnNumber) {

    std::string baseName =
        requestedName;

    if (baseName.empty()) {
        baseName =
            "Column_" +
            std::to_string(
                columnNumber + 1);
    }

    if (usedNames.insert(baseName).second) {
        return baseName;
    }

    std::size_t suffix = 2;

    while (true) {
        const std::string candidate =
            baseName +
            "_" +
            std::to_string(suffix);

        if (usedNames.insert(candidate).second) {
            return candidate;
        }

        ++suffix;
    }
}

bool parseWorksheet(
    const std::string& worksheetXml,
    const std::vector<std::string>& sharedStrings,
    const ExcelStyles& styles,
    bool uses1904DateSystem,
    DataTable& table,
    std::string& errorMessage) {

    XMLDocument document;

    if (document.Parse(
            worksheetXml.data(),
            worksheetXml.size()) != XML_SUCCESS) {

        errorMessage =
            "Unable to parse the first Excel worksheet.";

        return false;
    }

    XMLElement* root =
        document.RootElement();

    if (root == nullptr) {
        errorMessage =
            "Excel worksheet XML is empty.";

        return false;
    }

    XMLElement* sheetData =
        firstChildByLocalName(
            root,
            "sheetData");

    if (sheetData == nullptr) {
        errorMessage =
            "The Excel worksheet contains no sheet data.";

        return false;
    }

    std::vector<ParsedRow> rows;
    std::size_t maximumColumnCount = 0;

    for (XMLElement* rowElement =
             sheetData->FirstChildElement();
         rowElement != nullptr;
         rowElement =
             rowElement->NextSiblingElement()) {

        if (localName(rowElement->Name()) != "row") {
            continue;
        }

        ParsedRow parsedRow;
        std::size_t sequentialColumn = 0;

        for (XMLElement* cell =
                 rowElement->FirstChildElement();
             cell != nullptr;
             cell =
                 cell->NextSiblingElement()) {

            if (localName(cell->Name()) != "c") {
                continue;
            }

            std::size_t columnIndex =
                sequentialColumn;

            const char* reference =
                cell->Attribute("r");

            if (reference != nullptr) {
                columnIndex =
                    columnIndexFromReference(
                        reference);
            }

            parsedRow.push_back(
                CellData{
                    columnIndex,
                    parseCellValue(
                        cell,
                        sharedStrings,
                        styles,
                        uses1904DateSystem)
                });

            maximumColumnCount =
                std::max(
                    maximumColumnCount,
                    columnIndex + 1);

            sequentialColumn =
                columnIndex + 1;
        }

        rows.push_back(
            std::move(parsedRow));
    }

    if (rows.empty() ||
        maximumColumnCount == 0) {

        errorMessage =
            "The Excel worksheet is empty.";

        return false;
    }

    std::vector<DataValue> headerValues(
        maximumColumnCount);

    for (const CellData& cell : rows.front()) {
        if (cell.columnIndex <
            headerValues.size()) {

            headerValues[cell.columnIndex] =
                cell.value;
        }
    }

    std::set<std::string> usedColumnNames;

    for (std::size_t column = 0;
         column < maximumColumnCount;
         ++column) {

        std::string columnName =
            headerValues[column].toString();

        columnName =
            makeUniqueColumnName(
                columnName,
                usedColumnNames,
                column);

        table.addColumn(
            DataColumn(columnName));
    }

    for (std::size_t rowIndex = 1;
         rowIndex < rows.size();
         ++rowIndex) {

        std::vector<DataValue> values(
            maximumColumnCount);

        bool containsValue = false;

        for (const CellData& cell :
             rows[rowIndex]) {

            if (cell.columnIndex >=
                values.size()) {

                continue;
            }

            values[cell.columnIndex] =
                cell.value;

            if (!cell.value.isEmpty()) {
                containsValue = true;
            }
        }

        if (!containsValue) {
            continue;
        }

        DataRow row;

        for (std::size_t column = 0;
             column < maximumColumnCount;
             ++column) {

            row.addValue(
                values[column]);

            table.columnAt(column)
                .addValue(
                    values[column]);
        }

        table.addRow(row);
    }

    return true;
}

} // namespace

bool ExcelReader::read(
    const std::string& path,
    DataTable& table,
    std::string& errorMessage) {

    table.clear();
    errorMessage.clear();

    if (!std::filesystem::exists(path)) {
        errorMessage =
            "Excel file does not exist: " +
            path;

        return false;
    }

    const std::string extension =
        std::filesystem::path(path)
            .extension()
            .string();

    std::string lowerExtension;

    for (char character : extension) {
        lowerExtension +=
            static_cast<char>(
                std::tolower(
                    static_cast<unsigned char>(
                        character)));
    }

    if (lowerExtension != ".xlsx") {
        errorMessage =
            "ExcelReader supports the modern "
            ".xlsx format. Legacy .xls files "
            "are not supported.";

        return false;
    }

    std::vector<std::string> sharedStrings;

    if (!loadSharedStrings(
            path,
            sharedStrings,
            errorMessage)) {

        return false;
    }

    ExcelStyles styles;

    if (!loadStyles(
            path,
            styles,
            errorMessage)) {

        return false;
    }

    std::string worksheetPath;
    bool uses1904DateSystem = false;

    if (!getFirstWorksheetPath(
            path,
            worksheetPath,
            uses1904DateSystem,
            errorMessage)) {

        return false;
    }

    std::string worksheetXml;

    if (!readArchiveEntry(
            path,
            worksheetPath,
            worksheetXml,
            errorMessage)) {

        return false;
    }

    if (!parseWorksheet(
            worksheetXml,
            sharedStrings,
            styles,
            uses1904DateSystem,
            table,
            errorMessage)) {

        table.clear();
        return false;
    }

    return true;
}

} // namespace sdv
