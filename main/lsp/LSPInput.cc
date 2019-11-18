#include "main/lsp/LSPInput.h"
#include "common/FileOps.h"
#include "main/lsp/LSPMessage.h"
#include "spdlog/spdlog.h"
#include <iterator>

using namespace std;

namespace sorbet::realmain::lsp {

LSPFDInput::LSPFDInput(shared_ptr<spdlog::logger> logger, int inputFd) : logger(move(logger)), inputFd(inputFd) {}

unique_ptr<LSPMessage> LSPFDInput::read(int timeoutMs) {
    int length = -1;
    string allRead;
    {
        // Break and return if a timeout occurs. Bound loop to prevent infinite looping here. There's typically only two
        // lines in a header.
        for (int i = 0; i < 10; i += 1) {
            auto maybeLine = FileOps::readLineFromFd(inputFd, buffer, timeoutMs);
            if (!maybeLine) {
                // Line not read. Abort. Store what was read thus far back into buffer
                // for use in next call to function.
                buffer = absl::StrCat(allRead, buffer);
                return nullptr;
            }
            const string &line = *maybeLine;
            absl::StrAppend(&allRead, line, "\n");
            if (line == "\r") {
                // End of headers.
                break;
            }
            sscanf(line.c_str(), "Content-Length: %i\r", &length);
        }
        logger->trace("final raw read: {}, length: {}", allRead, length);
    }

    if (length < 0) {
        logger->trace("No \"Content-Length: %i\" header found.");
        // Throw away what we've read and start over.
        return nullptr;
    }

    if (buffer.length() < length) {
        // Need to read more.
        int moreNeeded = length - buffer.length();
        vector<char> buf(moreNeeded);
        int result = FileOps::readFd(inputFd, buf);
        if (result > 0) {
            buffer.append(buf.begin(), buf.begin() + result);
        }
        if (result == -1) {
            Exception::raise("Error reading file or EOF.");
        }
        if (result != moreNeeded) {
            // Didn't get enough data. Return read data to `buffer`.
            buffer = absl::StrCat(allRead, buffer);
            return nullptr;
        }
    }

    ENFORCE(buffer.length() >= length);

    string json = buffer.substr(0, length);
    buffer.erase(0, length);
    logger->debug("Read: {}\n", json);
    return LSPMessage::fromClient(json);
}

unique_ptr<LSPMessage> LSPProgrammaticInput::read(int timeoutMs) {
    absl::MutexLock lock(&mtx);
    if (closed && available.empty()) {
        throw sorbet::FileReadException("EOF");
    }

    mtx.AwaitWithTimeout(
        absl::Condition(
            +[](deque<unique_ptr<LSPMessage>> *available) -> bool { return !available->empty(); }, &available),
        absl::Milliseconds(timeoutMs));

    if (available.empty()) {
        return nullptr;
    }

    auto msg = move(available.front());
    available.pop_front();
    return msg;
}

void LSPProgrammaticInput::write(unique_ptr<LSPMessage> message) {
    absl::MutexLock lock(&mtx);
    if (closed) {
        Exception::raise("Cannot write to a closed input.");
    }
    available.push_back(move(message));
}

void LSPProgrammaticInput::write(vector<unique_ptr<LSPMessage>> messages) {
    absl::MutexLock lock(&mtx);
    available.insert(available.end(), make_move_iterator(messages.begin()), make_move_iterator(messages.end()));
}

void LSPProgrammaticInput::close() {
    absl::MutexLock lock(&mtx);
    if (closed) {
        Exception::raise("Input already ended.");
    }
    closed = true;
}

} // namespace sorbet::realmain::lsp
