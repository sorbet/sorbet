#include "json_types.h"

#include <string>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;

namespace sorbet::realmain::lsp {

std::string stringify(const rapidjson::Value &value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

DeserializationError::DeserializationError(const std::string &message)
    : std::runtime_error(fmt::format("Error deserializing JSON message: {}", message)) {}

InvalidStringEnumError::InvalidStringEnumError(const std::string &enumName, const std::string &value)
    : DeserializationError(fmt::format("Invalid value for enum type {}: {}", enumName, value)) {}

MissingFieldError::MissingFieldError(const std::string &objectName, const std::string &fieldName)
    : DeserializationError(fmt::format("Missing field {} on object of type {}.", fieldName, objectName)) {}

JSONTypeError::JSONTypeError(const std::string &fieldName, const std::string &expectedType)
    : DeserializationError(fmt::format("Expected field {} to have value of type {}.", fieldName, expectedType)) {}

JSONTypeError::JSONTypeError(const std::string &fieldName, const std::string &expectedType,
                             const rapidjson::Value &found)
    : DeserializationError(fmt::format("Expected field {} to have value of type {}, but had value `{}`.", fieldName,
                                       expectedType, stringify(found))) {}

JSONConstantError::JSONConstantError(const std::string &fieldName, const std::string &expectedValue,
                                     const rapidjson::Value &actualValue)
    : DeserializationError(fmt::format("Expected field {} to have value \"{}\", but had value `{}`.", fieldName,
                                       expectedValue, stringify(actualValue))) {}

SerializationError::SerializationError(const std::string &message)
    : std::runtime_error(fmt::format("Error serializing object to JSON message: {}", message)) {}

MissingVariantValueError::MissingVariantValueError(const std::string &fieldName)
    : SerializationError(fmt::format("Variant field {} does not have a value", fieldName)) {}

NullPtrError::NullPtrError(const std::string &fieldName)
    : SerializationError(fmt::format("Field {} does not have a value, and contains a null pointer", fieldName)) {}

InvalidEnumValueError::InvalidEnumValueError(const std::string &typeName, int value)
    : SerializationError(fmt::format("Found invalid value `{}` for enum of type {}", value, typeName)) {}

InvalidConstantValueError::InvalidConstantValueError(const std::string &fieldName, const std::string &expectedValue,
                                                     const std::string &actualValue)
    : SerializationError(fmt::format("Expected {} to have value \"{}\", but found value \"{}\".", fieldName,
                                     expectedValue, actualValue)) {}

InvalidTypeError::InvalidTypeError(const std::string &fieldName, const std::string &expectedType,
                                   const std::unique_ptr<rapidjson::Value> &found)
    : SerializationError(fmt::format("Expected field {} to have value of type {}, but had value `{}`.", fieldName,
                                     expectedType, stringify(*found))) {}

string JSONBaseType::toJSON() {
    std::unique_ptr<rapidjson::Document> d = std::make_unique<rapidjson::Document>();
    auto v = toJSONValueInternal(d);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v->Accept(writer);
    return buffer.GetString();
}

} // namespace sorbet::realmain::lsp
