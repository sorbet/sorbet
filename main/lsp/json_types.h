#ifndef RUBY_TYPER_LSP_JSON_TYPES_H
#define RUBY_TYPER_LSP_JSON_TYPES_H

#include "common/common.h"
#include "rapidjson/document.h"

#include <optional>
#include <variant>

namespace sorbet::realmain::lsp {

class DeserializationError : public std::runtime_error {
public:
    DeserializationError(std::string_view message);
};

class InvalidStringEnumError : public DeserializationError {
public:
    InvalidStringEnumError(std::string_view enumName, std::string_view value);
};

class MissingFieldError : public DeserializationError {
public:
    MissingFieldError(std::string_view objectName, std::string_view fieldName);
};

class JSONTypeError : public DeserializationError {
public:
    JSONTypeError(std::string_view fieldName, std::string_view expectedType);
    JSONTypeError(std::string_view fieldName, std::string_view expectedType, const rapidjson::Value &found);
};

class JSONConstantError : public DeserializationError {
public:
    JSONConstantError(std::string_view fieldName, std::string_view expectedValue, const rapidjson::Value &actualValue);
};

class SerializationError : public std::runtime_error {
public:
    SerializationError(std::string_view message);
};

class MissingVariantValueError : public SerializationError {
public:
    MissingVariantValueError(std::string_view fieldName);
};

class NullPtrError : public SerializationError {
public:
    NullPtrError(std::string_view fieldName);
};

class InvalidEnumValueError : public SerializationError {
public:
    InvalidEnumValueError(std::string_view typeName, int value);
};

class InvalidConstantValueError : public SerializationError {
public:
    InvalidConstantValueError(std::string_view fieldName, std::string_view expectedValue, std::string_view actualValue);
};

class InvalidTypeError : public SerializationError {
public:
    InvalidTypeError(std::string_view fieldName, std::string_view expectedType,
                     const std::unique_ptr<rapidjson::Value> &found);
};

class JSONNullObject {};

class JSONBaseType {
public:
    virtual ~JSONBaseType() = default;

    static const std::string defaultFieldName;

    /**
     * Converts C++ object into a string containing a stringified JSON object.
     */
    std::string toJSON() const;

    /**
     * Converts C++ object into a RapidJSON JSON value owned by the given rapidjson allocator.
     */
    virtual std::unique_ptr<rapidjson::Value> toJSONValue(rapidjson::MemoryPoolAllocator<> &alloc) const = 0;
};

#include "main/lsp/lsp_messages_gen.h"

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSP_JSON_TYPES_H
