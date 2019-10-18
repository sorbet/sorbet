#ifndef RUBY_TYPER_LSP_LSPOUTPUT_H
#define RUBY_TYPER_LSP_LSPOUTPUT_H

#include "absl/synchronization/mutex.h"
#include <memory>
#include <vector>

namespace spdlog {
class logger;
};

namespace sorbet::realmain::lsp {
class LSPMessage;
class ResponseMessage;
class NotificationMessage;

/**
 * Interface for sending messages back to the client.
 */
class LSPOutput {
    absl::Mutex mtx;

protected:
    virtual void rawWrite(std::unique_ptr<LSPMessage> msg) EXCLUSIVE_LOCKS_REQUIRED(mtx) = 0;

public:
    LSPOutput() = default;
    virtual ~LSPOutput() = default;
    /**
     * Write the given message to the output. Thread-safe via mutex.
     */
    void write(std::unique_ptr<LSPMessage> msg);
    void write(std::unique_ptr<ResponseMessage> msg);
    void write(std::unique_ptr<NotificationMessage> msg);
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

class LSPOutputToVector final : public LSPOutput {
    std::vector<std::unique_ptr<LSPMessage>> output;

protected:
    void rawWrite(std::unique_ptr<LSPMessage> msg) override;

public:
    LSPOutputToVector() = default;

    std::vector<std::unique_ptr<LSPMessage>> getOutput();
};

} // namespace sorbet::realmain::lsp

#endif
