#ifndef RUBY_TYPER_LSP_JSON_TYPES_H
#define RUBY_TYPER_LSP_JSON_TYPES_H

#include "common/common.h"
#include "rapidjson/document.h"

#include <optional>
#include <variant>

namespace sorbet::realmain::lsp {

class DeserializationError : public std::runtime_error {
public:
    DeserializationError(const std::string &message);
};

class InvalidStringEnumError : public DeserializationError {
public:
    InvalidStringEnumError(const std::string &enumName, const std::string &value);
};

class MissingFieldError : public DeserializationError {
public:
    MissingFieldError(const std::string &objectName, const std::string &fieldName);
};

class JSONTypeError : public DeserializationError {
public:
    JSONTypeError(const std::string &fieldName, const std::string &expectedType);
    JSONTypeError(const std::string &fieldName, const std::string &expectedType, const rapidjson::Value &found);
};

class JSONConstantError : public DeserializationError {
public:
    JSONConstantError(const std::string &fieldName, const std::string &expectedValue,
                      const rapidjson::Value &actualValue);
};

class SerializationError : public std::runtime_error {
public:
    SerializationError(const std::string &message);
};

class MissingVariantValueError : public SerializationError {
public:
    MissingVariantValueError(const std::string &fieldName);
};

class NullPtrError : public SerializationError {
public:
    NullPtrError(const std::string &fieldName);
};

class InvalidEnumValueError : public SerializationError {
public:
    InvalidEnumValueError(const std::string &typeName, int value);
};

class InvalidConstantValueError : public SerializationError {
public:
    InvalidConstantValueError(const std::string &fieldName, const std::string &expectedValue,
                              const std::string &actualValue);
};

class InvalidTypeError : public SerializationError {
public:
    InvalidTypeError(const std::string &fieldName, const std::string &expectedType,
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
    std::string toJSON();

    /**
     * Converts C++ object into a RapidJSON JSON value owned by the given rapidjson allocator.
     */
    virtual std::unique_ptr<rapidjson::Value> toJSONValue(rapidjson::MemoryPoolAllocator<> &alloc) const = 0;
};

#include "main/lsp/lsp_messages_gen.h"

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSP_JSON_TYPES_H
