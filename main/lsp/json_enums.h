#ifndef RUBY_TYPER_LSP_JSON_ENUMS_H
#define RUBY_TYPER_LSP_JSON_ENUMS_H

#include <string>

namespace sorbet::realmain::lsp {
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
    RequestFailed = -32803,
};

// Is not an enum, but is as simple as one. Putting it here reduces dependencies on json_types.h.
class JSONNullObject final {};

#include "main/lsp/lsp_messages_enums_gen.h"
} // namespace sorbet::realmain::lsp

#endif
