#ifndef RUBY_TYPER_LSP_JSON_TYPES_H
#define RUBY_TYPER_LSP_JSON_TYPES_H

#include "ast/ast.h"
#include "common/common.h"
#include "core/NameHash.h"
#include "core/core.h"
#include "rapidjson/document.h"

#include <optional>
#include <variant>

namespace sorbet::realmain::lsp {

/**
 * Encapsulates an update to LSP's file state in a compact form.
 * Placed into json_types.h because it is referenced from InitializedParams.
 */
class LSPFileUpdates final {
public:
    // This specific update contains edits with the given epoch
    u4 epoch = 0;
    // The total number of edits that this update represents. Used for stats.
    u4 editCount = 0;

    std::vector<std::shared_ptr<core::File>> updatedFiles;
    std::vector<core::FileHash> updatedFileHashes;
    std::vector<ast::ParsedFile> updatedFileIndexes;

    bool canTakeFastPath = false;
    // Indicates that this update contains a new file. Is a hack for determining if combining two updates can take the
    // fast path.
    bool hasNewFiles = false;
    // If true, this update caused a slow path to be canceled.
    bool canceledSlowPath = false;
    // Updated on typechecking thread. Contains indexes processed with typechecking global state.
    std::vector<ast::ParsedFile> updatedFinalGSFileIndexes;
    // (Optional) Updated global state object to use to typecheck this update.
    std::optional<std::unique_ptr<core::GlobalState>> updatedGS;
    // (Used in tests) Ensures that a slow path typecheck on these updates waits until it gets cancelled.
    bool cancellationExpected = false;
    // (Used in tests) Ensures that a slow path typecheck waits until this number of preemption occurs before finishing.
    int preemptionsExpected = 0;

    /**
     * Merges the given (and older) LSPFileUpdates object into this LSPFileUpdates object.
     *
     * Does not handle updating `canTakeFastPath`.
     */
    void mergeOlder(const LSPFileUpdates &older);

    /**
     * Returns a copy of this LSPFileUpdates object. Does not handle deepCopying `updatedGS`.
     */
    LSPFileUpdates copy() const;
};

enum class LSPErrorCodes {
    // Defined by JSON RPC
    ParseError = -32700,
    InvalidRequest = -32600,
    MethodNotFound = -32601,
    InvalidParams = -32602, // todo
    InternalError = -32603,
    ServerErrorStart = -32099,
    ServerErrorEnd = -32000,
    ServerNotInitialized = -32002,
    UnknownErrorCode = -32001,

    // Defined by the LSP
    RequestCancelled = -32800,
};

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

class JSONNullObject final {};

class JSONBaseType {
public:
    virtual ~JSONBaseType() = default;

    static const std::string defaultFieldName;

    /**
     * Converts C++ object into a string containing a stringified JSON object.
     */
    std::string toJSON(bool prettyPrint = false) const;

    /**
     * Converts C++ object into a RapidJSON JSON value owned by the given rapidjson allocator.
     */
    virtual std::unique_ptr<rapidjson::Value> toJSONValue(rapidjson::MemoryPoolAllocator<> &alloc) const = 0;
};

#include "main/lsp/lsp_messages_gen.h"

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSP_JSON_TYPES_H
