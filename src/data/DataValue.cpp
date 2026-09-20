#include "DataValue.h"

#include <sstream>

namespace sdv {

DataValue::DataValue()
    : type_(DataValueType::Empty),
      stringValue_(),
      numberValue_(0.0),
      booleanValue_(false) {
}

DataValue::DataValue(const std::string& value)
    : type_(DataValueType::String),
      stringValue_(value),
      numberValue_(0.0),
      booleanValue_(false) {
}

DataValue::DataValue(double value)
    : type_(DataValueType::Number),
      stringValue_(),
      numberValue_(value),
      booleanValue_(false) {
}

DataValue::DataValue(bool value)
    : type_(DataValueType::Boolean),
      stringValue_(),
      numberValue_(0.0),
      booleanValue_(value) {
}

DataValueType DataValue::type() const {
    return type_;
}

bool DataValue::isEmpty() const {
    return type_ == DataValueType::Empty;
}

bool DataValue::isString() const {
    return type_ == DataValueType::String;
}

bool DataValue::isNumber() const {
    return type_ == DataValueType::Number;
}

bool DataValue::isBoolean() const {
    return type_ == DataValueType::Boolean;
}

const std::string& DataValue::asString() const {
    return stringValue_;
}

double DataValue::asNumber() const {
    return numberValue_;
}

bool DataValue::asBoolean() const {
    return booleanValue_;
}

std::string DataValue::toString() const {
    switch (type_) {
        case DataValueType::Empty:
            return "";

        case DataValueType::String:
            return stringValue_;

        case DataValueType::Number: {
            std::ostringstream stream;
            stream << numberValue_;
            return stream.str();
        }

        case DataValueType::Boolean:
            return booleanValue_ ? "true" : "false";
    }

    return "";
}

} // namespace sdv
