#include "json_types.h"

#include <string>

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace std;

namespace sorbet::realmain::lsp {

string stringify(const rapidjson::Value &value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

DeserializationError::DeserializationError(const string &message)
    : runtime_error(fmt::format("Error deserializing JSON message: {}", message)) {}

InvalidStringEnumError::InvalidStringEnumError(const string &enumName, const std::string &value)
    : DeserializationError(fmt::format("Invalid value for enum type `{}`: {}", enumName, value)) {}

MissingFieldError::MissingFieldError(const string &objectName, const std::string &fieldName)
    : DeserializationError(fmt::format("Missing field `{}` on object of type `{}`.", fieldName, objectName)) {}

JSONTypeError::JSONTypeError(const string &fieldName, const std::string &expectedType)
    : DeserializationError(fmt::format("Expected field `{}` to have value of type `{}`.", fieldName, expectedType)) {}

JSONTypeError::JSONTypeError(const string &fieldName, const std::string &expectedType, const rapidjson::Value &found)
    : DeserializationError(fmt::format("Expected field `{}` to have value of type `{}`, but had value `{}`.", fieldName,
                                       expectedType, stringify(found))) {}

JSONConstantError::JSONConstantError(const string &fieldName, const std::string &expectedValue,
                                     const rapidjson::Value &actualValue)
    : DeserializationError(fmt::format("Expected field `{}` to have value \"{}\", but had value `{}`.", fieldName,
                                       expectedValue, stringify(actualValue))) {}

SerializationError::SerializationError(const string &message)
    : runtime_error(fmt::format("Error serializing object to JSON message: {}", message)) {}

MissingVariantValueError::MissingVariantValueError(const string &fieldName)
    : SerializationError(fmt::format("Variant field `{}` does not have a value", fieldName)) {}

NullPtrError::NullPtrError(const string &fieldName)
    : SerializationError(fmt::format("Field `{}` does not have a value, and contains a null pointer", fieldName)) {}

InvalidEnumValueError::InvalidEnumValueError(const string &typeName, int value)
    : SerializationError(fmt::format("Found invalid value `{}` for enum of type {}", value, typeName)) {}

InvalidConstantValueError::InvalidConstantValueError(const string &fieldName, const std::string &expectedValue,
                                                     const string &actualValue)
    : SerializationError(fmt::format("Expected `{}` to have value \"{}\", but found value \"{}\".", fieldName,
                                     expectedValue, actualValue)) {}

InvalidTypeError::InvalidTypeError(const string &fieldName, const std::string &expectedType,
                                   const unique_ptr<rapidjson::Value> &found)
    : SerializationError(fmt::format("Expected field `{}` to have value of type `{}`, but had value `{}`.", fieldName,
                                     expectedType, stringify(*found))) {}

const std::string JSONBaseType::defaultFieldName = "root";

string JSONBaseType::toJSON() {
    rapidjson::MemoryPoolAllocator<> alloc;
    auto v = toJSONValue(alloc);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v->Accept(writer);
    return buffer.GetString();
}

} // namespace sorbet::realmain::lsp
