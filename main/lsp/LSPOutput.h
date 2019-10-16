#ifndef RUBY_TYPER_LSP_LSPOUTPUT_H
#define RUBY_TYPER_LSP_LSPOUTPUT_H

#include <memory>

namespace sorbet::realmain::lsp {
class LSPMessage;

/**
 * Interface for sending messages back to the client.
 */
class LSPOutput {
protected:
    virtual void rawWrite(std::unique_ptr<LSPMessage> msg) = 0;

public:
    void write(std::unique_ptr<LSPMessage> msg);
};
} // namespace sorbet::realmain::lsp

#endif
