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
 * Interface for sending messages back to the client. It is thread-safe, so multiple threads can write concurrently to
 * the same output.
 */
class LSPOutput {
protected:
    absl::Mutex mtx;
    // Implementation-specific write implementation. Will be called from multiple threads, but invocations will never
    // interleave as method is protected by a mutex.
    virtual void rawWrite(std::unique_ptr<LSPMessage> msg) EXCLUSIVE_LOCKS_REQUIRED(mtx) = 0;

public:
    LSPOutput() = default;
    virtual ~LSPOutput() = default;
    /**
     * Write the given message to the output.
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
    void rawWrite(std::unique_ptr<LSPMessage> msg) override EXCLUSIVE_LOCKS_REQUIRED(mtx);

public:
    LSPStdout(std::shared_ptr<spdlog::logger> &logger);
};

/**
 * Appends all messages to an internal vector, which is emptied out when `getOutput()` is called.
 * Used in LSPWrapper and in tests.
 */
class LSPOutputToVector final : public LSPOutput {
    std::vector<std::unique_ptr<LSPMessage>> output GUARDED_BY(mtx);

protected:
    void rawWrite(std::unique_ptr<LSPMessage> msg) override EXCLUSIVE_LOCKS_REQUIRED(mtx);

public:
    LSPOutputToVector() = default;

    /**
     * Returns all written messages and empties internal vector.
     * That is, if called twice in a row without any intermediate writes, the second time it returns an empty vector.
     */
    std::vector<std::unique_ptr<LSPMessage>> getOutput();
};

} // namespace sorbet::realmain::lsp

#endif
