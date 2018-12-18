#ifndef RUBY_TYPER_LSP_JSON_TYPES_H
#define RUBY_TYPER_LSP_JSON_TYPES_H

#include "common/common.h"
#include "rapidjson/document.h"

#include <optional>
#include <variant>

using namespace std;

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

template <typename T> class JSONDocument {
public:
    // Owns all of the memory associated with JSONAny types, which are
    // represented as rapidjson::Value pointers.
    // An unfortunate necessity until we have smarter deserialization logic
    // that delurks all "Any" types.
    std::unique_ptr<rapidjson::Document> memoryOwner;

    // Root object extracted from the JSON document.
    std::unique_ptr<T> root;

    JSONDocument(std::unique_ptr<rapidjson::Document> &memoryOwner, std::unique_ptr<T> &root)
        : memoryOwner(std::move(memoryOwner)), root(std::move(root)){};

    /**
     * Casts the root object to the given type, and, if casting succeeds, moves owned items into a new
     * JSONDocument with the given root type.
     */
    template <typename NEW_TYPE> optional<unique_ptr<JSONDocument<NEW_TYPE>>> dynamicCast() {
        if (auto newRootPtr = dynamic_cast<NEW_TYPE *>(root.get())) {
            unique_ptr<NEW_TYPE> newRoot(newRootPtr);
            root.release();
            return make_unique<JSONDocument<NEW_TYPE>>(memoryOwner, newRoot);
        }
        // Cast failed.
        return nullopt;
    };
};

class JSONBaseType {
public:
    virtual ~JSONBaseType() = default;

    /**
     * Converts C++ object into a string containing a stringified JSON object.
     */
    std::string toJSON();

    /**
     * (For internal use only; public because it is used by codegen'd static methods.)
     * Converts C++ object into a RapidJSON JSON value owned by the given rapidjson document.
     */
    virtual std::unique_ptr<rapidjson::Value> toJSONValueInternal(const std::unique_ptr<rapidjson::Document> &d) = 0;

    /**
     * Converts C++ object into a rapidjson JSON object owned by the given JSONDocument.
     */
    template <typename T> std::unique_ptr<rapidjson::Value> toJSONValue(const std::unique_ptr<JSONDocument<T>> &doc) {
        return toJSONValueInternal(doc->memoryOwner);
    };
};

#include "main/lsp/lsp_messages_gen.h"

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSP_JSON_TYPES_H
