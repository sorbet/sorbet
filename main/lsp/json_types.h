#ifndef RUBY_TYPER_LSP_JSON_TYPES_H
#define RUBY_TYPER_LSP_JSON_TYPES_H

#include "common/Timer.h"
#include "common/common.h"
#include "core/core.h"
#include "main/lsp/json_enums.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"

#include <optional>
#include <variant>

namespace sorbet::realmain::lsp {

class DeserializationError : public std::runtime_error {
public:
    DeserializationError(std::string_view message);
};

class InvalidStringEnumError final : public DeserializationError {
public:
    const std::string enumName;
    const std::string value;
    InvalidStringEnumError(std::string_view enumName, std::string_view value);
};

class MissingFieldError final : public DeserializationError {
public:
    MissingFieldError(std::string_view fieldName);
};

class JSONTypeError final : public DeserializationError {
public:
    JSONTypeError(std::string_view fieldName, std::string_view expectedType);
    JSONTypeError(std::string_view fieldName, std::string_view expectedType, const rapidjson::Value &found);
};

class JSONConstantError final : public DeserializationError {
public:
    JSONConstantError(std::string_view fieldName, std::string_view expectedValue, const rapidjson::Value &actualValue);
};

class SerializationError : public std::runtime_error {
public:
    SerializationError(std::string_view message);
};

class MissingVariantValueError final : public SerializationError {
public:
    MissingVariantValueError(std::string_view fieldName);
};

class InvalidDiscriminantValueError final : public SerializationError {
public:
    const std::string fieldName;
    const std::string discriminantFieldName;
    const std::string discriminantValue;
    InvalidDiscriminantValueError(std::string_view fieldName, std::string_view discriminantFieldname,
                                  std::string_view discriminantValue);
};

class InvalidDiscriminatedUnionValueError final : public SerializationError {
public:
    const std::string fieldName;
    InvalidDiscriminatedUnionValueError(std::string_view fieldName, std::string_view discriminantFieldName,
                                        std::string_view discriminantValue, std::string_view expectedType);
};

class NullPtrError final : public SerializationError {
public:
    NullPtrError(std::string_view fieldName);
};

class InvalidEnumValueError final : public SerializationError {
public:
    InvalidEnumValueError(std::string_view typeName, int value);
};

class InvalidConstantValueError final : public SerializationError {
public:
    InvalidConstantValueError(std::string_view fieldName, std::string_view expectedValue, std::string_view actualValue);
};

class InvalidTypeError final : public SerializationError {
public:
    InvalidTypeError(std::string_view fieldName, std::string_view expectedType,
                     const std::unique_ptr<rapidjson::Value> &found);
};

class JSONBaseType {
public:
    virtual ~JSONBaseType() = default;

    static const std::string defaultFieldName;

    /**
     * Converts C++ object into a string containing a stringified JSON object.
     */
    std::string toJSON(bool prettyPrint = false) const;

    rapidjson::StringBuffer toJSONBuffer(bool prettyPrint = false) const;

    /**
     * Converts C++ object into a RapidJSON JSON value owned by the given rapidjson allocator.
     */
    virtual std::unique_ptr<rapidjson::Value> toJSONValue(rapidjson::MemoryPoolAllocator<> &alloc) const = 0;
};

#include "main/lsp/lsp_messages_gen.h"

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSP_JSON_TYPES_H
