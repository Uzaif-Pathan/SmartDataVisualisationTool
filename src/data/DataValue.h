#ifndef SMARTDATAVISUALISATION_DATAVALUE_H
#define SMARTDATAVISUALISATION_DATAVALUE_H

#include <string>

namespace sdv {

enum class DataValueType {
    Empty,
    String,
    Number,
    Boolean
};

class DataValue {
public:
    DataValue();
    explicit DataValue(const std::string& value);
    explicit DataValue(double value);
    explicit DataValue(bool value);

    DataValueType type() const;

    bool isEmpty() const;
    bool isString() const;
    bool isNumber() const;
    bool isBoolean() const;

    const std::string& asString() const;
    double asNumber() const;
    bool asBoolean() const;

    std::string toString() const;

private:
    DataValueType type_;
    std::string stringValue_;
    double numberValue_;
    bool booleanValue_;
};

} // namespace sdv

#endif // SMARTDATAVISUALISATION_DATAVALUE_H
