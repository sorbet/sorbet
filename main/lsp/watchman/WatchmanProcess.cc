#include "WatchmanProcess.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "common/strings/formatting.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "rapidjson/document.h"
#include "subprocess.hpp"

using namespace std;

namespace sorbet::realmain::lsp::watchman {

WatchmanProcess::WatchmanProcess(std::shared_ptr<spdlog::logger> logger, std::string_view watchmanPath,
                                 std::string_view workSpace, std::vector<std::string> extensions,
                                 MessageQueueState &messageQueue, absl::Mutex &messageQueueMutex,
                                 absl::Notification &initializedNotification,
                                 std::shared_ptr<const LSPConfiguration> config)
    : logger(std::move(logger)), watchmanPath(string(watchmanPath)), workSpace(string(workSpace)),
      extensions(std::move(extensions)),
      thread(runInAThread("watchmanReader", std::bind(&WatchmanProcess::start, this))), messageQueue(messageQueue),
      messageQueueMutex(messageQueueMutex), initializedNotification(initializedNotification),
      config(std::move(config)) {}

WatchmanProcess::~WatchmanProcess() {
    exitWithCode(0, "");
    // Destructor of Joinable ensures Watchman thread exits before this destructor finishes.
};

namespace {
template <typename F> void catchDeserializationError(spdlog::logger &logger, const string &line, F &&f) {
    try {
        f();
    } catch (sorbet::realmain::lsp::DeserializationError e) {
        // Gracefully handle deserialization errors, since they could be our fault.
        logger.error("Unable to deserialize Watchman request: {}\nOriginal request:\n{}", e.what(), line);
    }
}

} // namespace

optional<WatchmanProcess::ReadResponse> WatchmanProcess::readResponse(FILE *file, int fd, string &buffer) {
    errno = 0;
    auto maybeLine = FileOps::readLineFromFd(fd, buffer);
    if (maybeLine.result == FileOps::ReadResult::Timeout) {
        // Timeout occurred. See if we should abort before reading further.
        return nullopt;
    }

    if (maybeLine.result == FileOps::ReadResult::ErrorOrEof) {
        if (errno == EINTR) {
            return nullopt;
        }

        // Exit loop; unable to read from Watchman process.
        exitWithCode(1, nullopt);
        return nullopt;
    }

    ENFORCE(maybeLine.result == FileOps::ReadResult::Success);

    if (!maybeLine.output.has_value()) {
        return nullopt;
    }

    auto line = move(maybeLine.output.value());
    rapidjson::MemoryPoolAllocator<> alloc;
    rapidjson::Document d(&alloc);
    logger->debug(line);
    if (d.Parse(line.c_str(), line.size()).HasParseError()) {
        logger->error("Error parsing Watchman response: `{}` is not a valid json object", line);
        return nullopt;
    }

    return make_optional<ReadResponse>(move(line), move(d));
}

void WatchmanProcess::start() {
    auto mainPid = getpid();
    try {
        string subscriptionName = fmt::format("sorbet-{}", getpid());

        logger->debug("Starting monitoring path {} with watchman for files with extensions {}. Subscription id: {}",
                      workSpace, fmt::join(extensions, ","), subscriptionName);

        auto p = subprocess::Popen({watchmanPath.c_str(), "-j", "-p", "--no-pretty"},
                                   subprocess::output{subprocess::PIPE}, subprocess::input{subprocess::PIPE});

        auto file = p.output();
        auto fd = fileno(file);

        string buffer;

        // Note: Newer versions of Watchman (post 4.9.0) support ["suffix", ["suffix1", "suffix2", ...]], but Stripe
        // laptops have 4.9.0. Thus, we use [ "anyof", [ "suffix", "suffix1" ], [ "suffix", "suffix2" ], ... ].
        // Note 2: `empty_on_fresh_instance` prevents Watchman from sending entire contents of folder if this
        // subscription starts the daemon / causes the daemon to watch this folder for the first time.
        string subscribeCommand = fmt::format(
            "["
            /**/ "\"subscribe\", "
            /**/ "\"{}\", "
            /**/ "\"{}\", "
            /**/ "{{"
            /*    */ "\"expression\": ["
            /*        */ "\"allof\", "
            /*        */ "[\"type\", \"f\"], "
            /*        */ "[\"anyof\", {}], "
            /*        */ // Exclude rsync tmpfiles
            /*        */ "[\"not\", [\"match\", \"**/.~tmp~/**\", \"wholename\", {{\"includedotfiles\": true}}]]"
            /*    */ "], "
            /*    */ "\"fields\": [\"name\"], "
            /*    */ "\"empty_on_fresh_instance\": true"
            /**/ "}}"
            "]",
            workSpace, subscriptionName, fmt::map_join(extensions, ", ", [](const std::string &ext) -> string {
                return fmt::format("[\"suffix\", \"{}\"]", ext);
            }));
        p.send(subscribeCommand.c_str(), subscribeCommand.size());
        logger->debug(subscribeCommand);

        if (auto res = readResponse(file, fd, buffer)) {
            if (!res->d.HasMember("subscribe")) {
                // Something we don't understand yet.
                logger->debug("Unknown Watchman response:\n{}", res->line);
            }
        }

        while (!isStopped()) {
            ReadResponse res;
            if (auto maybeRes = readResponse(file, fd, buffer)) {
                res = move(maybeRes.value());
            } else {
                continue;
            }

            auto line = move(res.line);
            auto d = move(res.d);

            if (d.HasMember("is_fresh_instance")) {
                catchDeserializationError(*logger, line, [&d, this]() {
                    auto queryResponse = sorbet::realmain::lsp::WatchmanQueryResponse::fromJSONValue(d);
                    processQueryResponse(move(queryResponse));
                });
            } else if (d.HasMember("state-enter")) {
                // These are messages from "state-enter" commands.  See
                // https://facebook.github.io/watchman/docs/cmd/state-enter.html
                // for more information.
                catchDeserializationError(*logger, line, [&d, this]() {
                    auto stateEnter = sorbet::realmain::lsp::WatchmanStateEnter::fromJSONValue(d);
                    processStateEnter(move(stateEnter));
                });
            } else if (d.HasMember("state-leave")) {
                // These are messages from "state-leave" commands.  See
                // https://facebook.github.io/watchman/docs/cmd/state-leave.html
                // for more information.
                catchDeserializationError(*logger, line, [&d, this]() {
                    auto stateLeave = sorbet::realmain::lsp::WatchmanStateLeave::fromJSONValue(d);
                    processStateLeave(move(stateLeave));
                });
            } else {
                // Something we don't understand yet.
                logger->debug("Unknown Watchman response:\n{}", line);
            }
        }
    } catch (exception e) {
        // Ignore exceptions thrown on forked process.
        if (getpid() == mainPid) {
            auto msg = fmt::format(
                "Error running Watchman (with `{} -j -p --no-pretty`).\nWatchman is required for Sorbet to "
                "detect changes to files made outside of your code editor.\nDon't need Watchman? Run Sorbet "
                "with `--disable-watchman`.",
                watchmanPath);
            logger->error(msg);
            exitWithCode(1, msg);
        } else {
            // The forked process failed to start, likely because Watchman wasn't found. Exit the process.
            exit(1);
        }
    }

    ENFORCE(isStopped());
}

bool WatchmanProcess::isStopped() {
    absl::MutexLock lck(&mutex);
    return stopped;
}

void WatchmanProcess::exitWithCode(int code, const std::optional<std::string> &msg) {
    absl::MutexLock lck(&mutex);
    if (!stopped) {
        stopped = true;
        processExit(code, msg);
    }
}

void WatchmanProcess::enqueueNotification(std::unique_ptr<NotificationMessage> notification) {
    auto msg = make_unique<LSPMessage>(move(notification));
    // Don't start enqueueing requests until LSP is initialized.
    initializedNotification.WaitForNotification();
    {
        absl::MutexLock lck(&messageQueueMutex);
        msg->tagNewRequest(*logger);
        messageQueue.counters = mergeCounters(move(messageQueue.counters));
        messageQueue.pendingRequests.push_back(move(msg));
    }
}

void WatchmanProcess::processQueryResponse(std::unique_ptr<WatchmanQueryResponse> response) {
    auto notifMsg = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(response));
    enqueueNotification(move(notifMsg));
}

void WatchmanProcess::processStateEnter(std::unique_ptr<sorbet::realmain::lsp::WatchmanStateEnter> stateEnter) {
    auto notification = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanStateEnter, move(stateEnter));
    enqueueNotification(move(notification));
}

void WatchmanProcess::processStateLeave(std::unique_ptr<sorbet::realmain::lsp::WatchmanStateLeave> stateLeave) {
    auto notification = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanStateLeave, move(stateLeave));
    enqueueNotification(move(notification));
}

void WatchmanProcess::processExit(int watchmanExitCode, const std::optional<std::string> &msg) {
    {
        absl::MutexLock lck(&messageQueueMutex);
        if (!messageQueue.terminate) {
            messageQueue.terminate = true;
            messageQueue.errorCode = watchmanExitCode;
            if (watchmanExitCode != 0 && msg.has_value()) {
                auto params = make_unique<ShowMessageParams>(MessageType::Error, msg.value());
                config->output->write(make_unique<LSPMessage>(
                    make_unique<NotificationMessage>("2.0", LSPMethod::WindowShowMessage, move(params))));
            }
        }
        logger->debug("Watchman terminating");
    }
}

} // namespace sorbet::realmain::lsp::watchman
