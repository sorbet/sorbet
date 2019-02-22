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

DeserializationError::DeserializationError(string_view message)
    : runtime_error(fmt::format("Error deserializing JSON message: {}", message)) {}

InvalidStringEnumError::InvalidStringEnumError(string_view enumName, string_view value)
    : DeserializationError(fmt::format("Invalid value for enum type `{}`: {}", enumName, value)) {}

MissingFieldError::MissingFieldError(string_view objectName, string_view fieldName)
    : DeserializationError(fmt::format("Missing field `{}` on object of type `{}`.", fieldName, objectName)) {}

JSONTypeError::JSONTypeError(string_view fieldName, string_view expectedType)
    : DeserializationError(fmt::format("Expected field `{}` to have value of type `{}`.", fieldName, expectedType)) {}

JSONTypeError::JSONTypeError(string_view fieldName, string_view expectedType, const rapidjson::Value &found)
    : DeserializationError(fmt::format("Expected field `{}` to have value of type `{}`, but had value `{}`.", fieldName,
                                       expectedType, stringify(found))) {}

JSONConstantError::JSONConstantError(string_view fieldName, string_view expectedValue,
                                     const rapidjson::Value &actualValue)
    : DeserializationError(fmt::format("Expected field `{}` to have value \"{}\", but had value `{}`.", fieldName,
                                       expectedValue, stringify(actualValue))) {}

SerializationError::SerializationError(string_view message)
    : runtime_error(fmt::format("Error serializing object to JSON message: {}", message)) {}

MissingVariantValueError::MissingVariantValueError(string_view fieldName)
    : SerializationError(fmt::format("Variant field `{}` does not have a value", fieldName)) {}

NullPtrError::NullPtrError(string_view fieldName)
    : SerializationError(fmt::format("Field `{}` does not have a value, and contains a null pointer", fieldName)) {}

InvalidEnumValueError::InvalidEnumValueError(string_view typeName, int value)
    : SerializationError(fmt::format("Found invalid value `{}` for enum of type {}", value, typeName)) {}

InvalidConstantValueError::InvalidConstantValueError(string_view fieldName, string_view expectedValue,
                                                     string_view actualValue)
    : SerializationError(fmt::format("Expected `{}` to have value \"{}\", but found value \"{}\".", fieldName,
                                     expectedValue, actualValue)) {}

InvalidTypeError::InvalidTypeError(string_view fieldName, string_view expectedType,
                                   const unique_ptr<rapidjson::Value> &found)
    : SerializationError(fmt::format("Expected field `{}` to have value of type `{}`, but had value `{}`.", fieldName,
                                     expectedType, stringify(*found))) {}

const std::string JSONBaseType::defaultFieldName = "root";

string JSONBaseType::toJSON() const {
    rapidjson::MemoryPoolAllocator<> alloc;
    auto v = toJSONValue(alloc);
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    v->Accept(writer);
    return buffer.GetString();
}

} // namespace sorbet::realmain::lsp
