#ifndef RUBY_TYPER_LSP_SHOWOPERATION_H
#define RUBY_TYPER_LSP_SHOWOPERATION_H

#include <string>
#include <string_view>

namespace sorbet::realmain::lsp {
class LSPOutput;
class LSPConfiguration;

/**
 * Object that uses the RAII pattern to notify the client when a *slow* operation
 * starts and ends. Is used to provide user feedback in the status line of VS Code.
 */
class ShowOperation final {
private:
    LSPOutput &output;
    const LSPConfiguration &config;
    const std::string operationName;
    const std::string description;

public:
    ShowOperation(LSPOutput &output, const LSPConfiguration &config, std::string_view operationName,
                  std::string_view description);
    ~ShowOperation();
};
} // namespace sorbet::realmain::lsp

#endif
