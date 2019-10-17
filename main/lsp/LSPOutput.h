#ifndef RUBY_TYPER_LSP_LSPOUTPUT_H
#define RUBY_TYPER_LSP_LSPOUTPUT_H

#include <memory>

namespace spdlog {
class logger;
};

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
    // Used for debug output.
    std::shared_ptr<spdlog::logger> logger;

protected:
    void rawWrite(std::unique_ptr<LSPMessage> msg) override;

public:
    LSPStdout(std::shared_ptr<spdlog::logger> &logger);
};

} // namespace sorbet::realmain::lsp

#endif
