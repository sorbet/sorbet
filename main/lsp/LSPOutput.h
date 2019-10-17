#ifndef RUBY_TYPER_LSP_LSPOUTPUT_H
#define RUBY_TYPER_LSP_LSPOUTPUT_H

#include <functional>
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
    LSPOutput() = default;
    virtual ~LSPOutput() = default;
    void write(std::unique_ptr<LSPMessage> msg);
};

/**
 * An implementation of LSPOutput that writes to stdout.
 */
class LSPStdout final : public LSPOutput {
    std::function<void(std::string)> log;

protected:
    void rawWrite(std::unique_ptr<LSPMessage> msg) override;

public:
    LSPStdout(std::function<void(std::string)> log);
};

} // namespace sorbet::realmain::lsp

#endif
