#ifndef RUBY_TYPER_LSP_LSPINPUT_H
#define RUBY_TYPER_LSP_LSPINPUT_H

#include "absl/synchronization/mutex.h"
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
    LSPInput() = default;
    virtual ~LSPInput() = default;
    /**
     * Blockingly reads the next message from the client with the given timeout (in milliseconds). Returns nullptr if no
     * message read within timeout.
     *
     * Throws a FileReadException on error or EOF.
     */
    virtual std::unique_ptr<LSPMessage> read(int timeoutMs = 100) = 0;
};

/**
 * Reads messages from a file descriptor (like stdin).
 *
 * Throws a FileReadException on error or EOF.
 */
class LSPFDInput final : public LSPInput {
    // Contains unparsed strings containing a partial message read from file descriptor.
    std::string buffer;
    // Used to log debug messages.
    const std::shared_ptr<spdlog::logger> logger;

public:
    const int inputFd;
    LSPFDInput(std::shared_ptr<spdlog::logger> logger, int inputFd);

    std::unique_ptr<LSPMessage> read(int timeoutMs) override;
};

/**
 * Input is provided programmatically via the `write` method. Threadsafe.
 *
 * Throws a FileReadException when stream has been closed.
 */
class LSPProgrammaticInput final : public LSPInput {
    absl::Mutex mtx;
    // Contains all available messages for processing.
    std::deque<std::unique_ptr<LSPMessage>> available GUARDED_BY(mtx);
    bool closed GUARDED_BY(mtx) = false;

public:
    LSPProgrammaticInput() = default;

    std::unique_ptr<LSPMessage> read(int timeoutMs) override;

    /** Send the given message to input. */
    void write(std::unique_ptr<LSPMessage> message);

    /** Send the given messages to input. */
    void write(std::vector<std::unique_ptr<LSPMessage>> messages);

    /** Closes the input. Causes an EOF to be thrown after all existing messages are read. */
    void close();
};

} // namespace sorbet::realmain::lsp

#endif
