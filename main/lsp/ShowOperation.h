#ifndef RUBY_TYPER_LSP_SHOWOPERATION_H
#define RUBY_TYPER_LSP_SHOWOPERATION_H

#include <string>

namespace sorbet::realmain::lsp {
class LSPOutput;
class LSPConfiguration;

/**
 * Object that uses the RAII pattern to notify the client when a *slow* operation
 * starts and ends. Is used to provide user feedback in the status line of VS Code.
 */
class ShowOperation final {
    const LSPConfiguration &config;
    const std::string operationName;
    const std::string description;

public:
    enum class Kind {
        Indexing = 1,
        SlowPathBlocking,
        SlowPathNonBlocking,
        FastPath,
        References,
        SymbolSearch,
        Rename,
        MoveMethod,
    };
    ShowOperation(const LSPConfiguration &config, Kind kind);
    ~ShowOperation();
};
} // namespace sorbet::realmain::lsp

#endif
