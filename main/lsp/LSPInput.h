#ifndef RUBY_TYPER_LSP_LSPINPUT_H
#define RUBY_TYPER_LSP_LSPINPUT_H

#include "absl/synchronization/mutex.h"
#include "common/FileOps.h"
#include <deque>
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
 * Interface for receiving messages from the client. It is _not_ thread safe, and assumes that only one thread will be
 * reading from it at a time.
 */
class LSPInput {
public:
    struct ReadOutput {
        FileOps::ReadResult result;
        std::unique_ptr<LSPMessage> message;
    };

    LSPInput() = default;
    virtual ~LSPInput() = default;
    /**
     * Blockingly reads the next message from the client with the given timeout (in milliseconds). Returns an output
     * object that indicates if the read succeeded, timed out, or failed.
     */
    virtual ReadOutput read(int timeoutMs = 100) = 0;
};

/**
 * Reads messages from a file descriptor (like stdin).
 */
class LSPFDInput final : public LSPInput {
    // Contains unparsed strings containing a partial message read from file descriptor.
    std::string buffer;
    // Used to log debug messages.
    const std::shared_ptr<spdlog::logger> logger;

public:
    const int inputFd;
    LSPFDInput(std::shared_ptr<spdlog::logger> logger, int inputFd);

    ReadOutput read(int timeoutMs) override;
};

/**
 * Input is provided programmatically via the `write` method. Threadsafe.
 */
class LSPProgrammaticInput final : public LSPInput {
    absl::Mutex mtx;
    // Contains all available messages for processing.
    std::deque<std::unique_ptr<LSPMessage>> available ABSL_GUARDED_BY(mtx);
    bool closed ABSL_GUARDED_BY(mtx) = false;

public:
    LSPProgrammaticInput() = default;

    ReadOutput read(int timeoutMs) override;

    /** Send the given message to input. */
    void write(std::unique_ptr<LSPMessage> message);

    /** Send the given messages to input. */
    void write(std::vector<std::unique_ptr<LSPMessage>> messages);

    /** Closes the input. Causes an EOF to be thrown after all existing messages are read. */
    void close();
};

} // namespace sorbet::realmain::lsp

#endif
