#include "WatchmanProcess.h"
#include "WatchmanRestartPolicy.h"
#include "WatchmanShutdown.h"
#include "WatchmanSubscription.h"
#include "absl/strings/strip.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "common/strings/formatting.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "rapidjson/document.h"
#include "subprocess.hpp"
#include <thread>

using namespace std;

namespace sorbet::realmain::lsp::watchman {

WatchmanProcess::WatchmanProcess(shared_ptr<spdlog::logger> logger, string_view watchmanPath, string_view workSpace,
                                 vector<string> extensions, MessageQueueState &messageQueue,
                                 absl::Mutex &messageQueueMutex, absl::Notification &initializedNotification,
                                 shared_ptr<const LSPConfiguration> config, string_view watchmanNamespace)
    : logger(std::move(logger)), watchmanPath(string(watchmanPath)), workSpace(string(workSpace)),
      extensions(std::move(extensions)),
      thread(runInAThread("watchmanReader", std::bind(&WatchmanProcess::start, this))), messageQueue(messageQueue),
      messageQueueMutex(messageQueueMutex), initializedNotification(initializedNotification), config(std::move(config)),
      watchmanNamespace(watchmanNamespace) {}

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

// The clock a watchman response was generated at, if it carries one.
optional<string> clockOf(const rapidjson::Document &d) {
    auto clock = d.FindMember("clock");
    if (clock == d.MemberEnd() || !clock->value.IsString()) {
        return nullopt;
    }
    return string(clock->value.GetString(), clock->value.GetStringLength());
}

// Deserializes a file change response, stripping the namespace prefix off its paths. Returns nullptr if it could not
// be deserialized, having logged why.
unique_ptr<sorbet::realmain::lsp::WatchmanQueryResponse> parseQueryResponse(spdlog::logger &logger,
                                                                            const rapidjson::Document &d,
                                                                            const string &line,
                                                                            string_view watchmanNamespace) {
    unique_ptr<sorbet::realmain::lsp::WatchmanQueryResponse> parsed;
    catchDeserializationError(logger, line, [&]() {
        auto queryResponse = sorbet::realmain::lsp::WatchmanQueryResponse::fromJSONValue(d);
        if (!watchmanNamespace.empty()) {
            string prefix(watchmanNamespace);
            prefix += "/";

            for (auto &file : queryResponse->files) {
                string_view view(file);
                if (!absl::ConsumePrefix(&view, prefix)) {
                    continue;
                }
                file = view;
            }
        }
        parsed = move(queryResponse);
    });
    return parsed;
}

// Reaps a watchman child on every exit path out of a scope, including an exception. Declare after the Popen so that it
// only runs if construction succeeded.
struct ShutdownGuard {
    subprocess::Popen &p;
    ~ShutdownGuard() noexcept {
        shutdownWatchmanChild(p);
    }
};

// Generous: giving up here leaves Sorbet stale on whatever changed while it had no subscription, until those files
// change again.
constexpr chrono::seconds CATCH_UP_TIMEOUT{120};

} // namespace

void WatchmanProcess::start() {
    auto mainPid = getpid();
    try {
        string subscriptionName = fmt::format("ruby-typer-{}", getpid());
        string root = resolveWatchmanRoot();

        WatchmanRestartPolicy restartPolicy;
        while (!isStopped()) {
            auto startedAt = chrono::steady_clock::now();
            auto session = runSubscription(root, subscriptionName);
            auto ranFor = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - startedAt);

            if (session.outcome == SessionOutcome::Stopped || isStopped()) {
                break;
            }

            auto delay = restartPolicy.delayBeforeRestart(ranFor, session.subscribed);
            if (!delay.has_value()) {
                auto msg = fmt::format(
                    "Lost the connection to Watchman (`{} -j -p --no-pretty`) {} times in a row without it holding a "
                    "working subscription; the last lasted {}ms and was{} acknowledged.\nWatchman is required for "
                    "Sorbet to detect changes to files made outside of your code editor.\nDon't need Watchman? Run "
                    "Sorbet with `--disable-watchman`.",
                    watchmanPath, restartPolicy.unhealthySessionCount(), ranFor.count(),
                    session.subscribed ? "" : " never");
                logger->error(msg);
                exitWithCode(1, msg);
                break;
            }

            // At error level on purpose: this is the first line to look for if an edit around this time went
            // unnoticed.
            logger->error("Lost the connection to Watchman after {}ms. Subscribing again in {}ms.", ranFor.count(),
                          delay->count());
            sleepUnlessStopped(*delay);
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

string WatchmanProcess::resolveWatchmanRoot() {
    string modifiedWorkspace = workSpace;
    if (!watchmanNamespace.empty()) {
        const optional<string> maybeResolved = FileOps::realpath(workSpace);
        if (!maybeResolved.has_value()) {
            logger->debug("Unable to resolve workspace path {} for namespacing", workSpace);
            watchmanNamespace.clear();
        } else {
            string_view root(*maybeResolved);
            logger->debug("realpath({}) = {}", workSpace, root);
            if (absl::ConsumeSuffix(&root, watchmanNamespace)) {
                string gitDirectory(root);
                gitDirectory += "/.git";
                if (FileOps::dirExists(gitDirectory)) {
                    logger->debug("Using {} as watchman root", root);
                    modifiedWorkspace = root;
                } else {
                    logger->debug("Parent directory {} of namespace {} is not a git repository, disabling namespacing",
                                  root, watchmanNamespace);
                    watchmanNamespace.clear();
                }
            } else {
                logger->debug("Watched directory {} is not in namespace {}, disabling namespacing", root,
                              watchmanNamespace);
                watchmanNamespace.clear();
            }
        }
    }
    return modifiedWorkspace;
}

WatchmanProcess::SessionResult WatchmanProcess::runSubscription(string_view root, string_view subscriptionName) {
    bool subscribed = false;

    // Captured before this session can advance it. Empty for the first subscription of the process, where there is no
    // window to catch up on: Sorbet is about to read the tree itself.
    const optional<string> resumeFrom = lastClock;

    auto p = subprocess::Popen({watchmanPath.c_str(), "-j", "-p", "--no-pretty"}, subprocess::output{subprocess::PIPE},
                               subprocess::input{subprocess::PIPE});

    ShutdownGuard shutdownGuard{p};

    logger->debug("Starting monitoring path {} with watchman for files with extensions {}. Subscription id: {}", root,
                  fmt::join(extensions, ","), subscriptionName);

    string subscribeCommand = buildSubscribeCommand(root, subscriptionName, extensions, watchmanNamespace);
    p.send(subscribeCommand.c_str(), subscribeCommand.size());
    logger->debug(subscribeCommand);

    if (resumeFrom.has_value()) {
        // A subscription only reports what changes once watchman creates it, so the window before that has to be asked
        // for separately. Overlapping the two costs a redundant file read; a gap between them costs Sorbet's view of
        // whatever changed in it.
        catchUpSince(root, *resumeFrom);
    }

    auto file = p.output();
    auto fd = fileno(file);

    // Per-session on purpose: prepending half of a dead session's response to the first of the next would not parse.
    string buffer;

    while (!isStopped()) {
        releaseHeldNotifications();

        errno = 0;
        auto maybeLine = FileOps::readLineFromFd(fd, buffer);
        if (maybeLine.result == FileOps::ReadResult::Timeout) {
            // Timeout occurred. See if we should abort before reading further.
            continue;
        }

        if (maybeLine.result == FileOps::ReadResult::ErrorOrEof) {
            if (errno == EINTR) {
                continue;
            }

            // Either the CLI exited or the daemon closed the socket under it. Neither means watchman is unusable.
            return {SessionOutcome::Disconnected, subscribed};
        }

        ENFORCE(maybeLine.result == FileOps::ReadResult::Success);

        const string &line = *maybeLine.output;
        // Line found!
        rapidjson::MemoryPoolAllocator<> alloc;
        rapidjson::Document d(&alloc);
        logger->debug(line);
        if (d.Parse(line.c_str(), line.size()).HasParseError()) {
            logger->error("Error parsing Watchman response: `{}` is not a valid json object", line);
        } else if (d.HasMember("is_fresh_instance")) {
            rememberClock(clockOf(d));
            if (auto queryResponse = parseQueryResponse(*logger, d, line, watchmanNamespace);
                queryResponse != nullptr) {
                processQueryResponse(move(queryResponse));
            }
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
        } else if (d.HasMember("subscribe")) {
            // The ack. Its clock is where a replacement subscription resumes from if this one dies before any file
            // changes arrive.
            subscribed = true;
            rememberClock(clockOf(d));
        } else {
            // Something we don't understand yet.
            logger->debug("Unknown Watchman response:\n{}", line);
        }
    }

    return {SessionOutcome::Stopped, subscribed};
}

void WatchmanProcess::catchUpSince(string_view root, string_view sinceClock) {
    string queryCommand = buildCatchUpQueryCommand(root, extensions, watchmanNamespace, sinceClock);

    // A connection of its own: `-j` reads one command per process, so this cannot share the subscription's, and
    // without `-p` it answers once and exits.
    auto p = subprocess::Popen({watchmanPath.c_str(), "-j", "--no-pretty"}, subprocess::output{subprocess::PIPE},
                               subprocess::input{subprocess::PIPE});
    ShutdownGuard shutdownGuard{p};

    p.send(queryCommand.c_str(), queryCommand.size());
    logger->debug(queryCommand);

    auto fd = fileno(p.output());
    string buffer;
    const auto deadline = chrono::steady_clock::now() + CATCH_UP_TIMEOUT;

    while (!isStopped() && chrono::steady_clock::now() < deadline) {
        errno = 0;
        auto maybeLine = FileOps::readLineFromFd(fd, buffer);
        if (maybeLine.result == FileOps::ReadResult::Timeout) {
            continue;
        }

        if (maybeLine.result == FileOps::ReadResult::ErrorOrEof) {
            if (errno == EINTR) {
                continue;
            }
            logger->error("Watchman exited without reporting the changes Sorbet missed since {}. Sorbet may be out of "
                          "date on files that changed while it was not watching, until they change again.",
                          sinceClock);
            return;
        }

        const string &line = *maybeLine.output;
        rapidjson::MemoryPoolAllocator<> alloc;
        rapidjson::Document d(&alloc);
        logger->debug(line);
        if (d.Parse(line.c_str(), line.size()).HasParseError()) {
            logger->error("Error parsing Watchman response: `{}` is not a valid json object", line);
            return;
        }

        if (!d.HasMember("is_fresh_instance")) {
            // An `error` response, most likely: the clock was rejected, or the root is no longer watched.
            logger->error("Watchman could not report the changes Sorbet missed since {}: {}", sinceClock, line);
            return;
        }

        rememberClock(clockOf(d));
        if (auto queryResponse = parseQueryResponse(*logger, d, line, watchmanNamespace); queryResponse != nullptr) {
            logger->debug("Caught up on {} file(s) changed since {}", queryResponse->files.size(), sinceClock);
            processQueryResponse(move(queryResponse));
        }
        return;
    }

    if (!isStopped()) {
        logger->error("Timed out after {}s asking Watchman for the changes Sorbet missed since {}. Sorbet may be out "
                      "of date on files that changed while it was not watching, until they change again.",
                      CATCH_UP_TIMEOUT.count(), sinceClock);
    }
}

void WatchmanProcess::rememberClock(optional<string> clock) {
    if (clock.has_value()) {
        lastClock = move(clock);
    }
}

void WatchmanProcess::sleepUnlessStopped(chrono::milliseconds duration) {
    // Sliced so that a shutdown arriving mid-delay does not have to wait the delay out.
    constexpr chrono::milliseconds slice{20};
    for (auto remaining = duration; remaining > chrono::milliseconds{0}; remaining -= slice) {
        if (isStopped()) {
            return;
        }
        this_thread::sleep_for(remaining < slice ? remaining : slice);
    }
}

bool WatchmanProcess::isStopped() {
    absl::MutexLock lck(&mutex);
    return stopped;
}

void WatchmanProcess::exitWithCode(int code, const optional<string> &msg) {
    absl::MutexLock lck(&mutex);
    if (!stopped) {
        stopped = true;
        processExit(code, msg);
    }
}

void WatchmanProcess::enqueueNotification(unique_ptr<NotificationMessage> notification) {
    auto msg = make_unique<LSPMessage>(move(notification));

    // Sorbet cannot use these until the LSP loop is initialized, but waiting for that here would stop this thread
    // draining the CLI, and watchman drops a client that has not accepted its output for 60 seconds (`kWriteTimeout`).
    // The initial typecheck of a large codebase takes minutes, so hold them and keep reading instead.
    if (!initializedNotification.HasBeenNotified()) {
        heldUntilInitialized.push_back(move(msg));
        return;
    }

    // Held notifications first, or this one overtakes them.
    releaseHeldNotifications();

    absl::MutexLock lck(&messageQueueMutex);
    msg->tagNewRequest(*logger);
    messageQueue.counters = mergeCounters(move(messageQueue.counters));
    messageQueue.pendingRequests.push_back(move(msg));
}

void WatchmanProcess::releaseHeldNotifications() {
    if (heldUntilInitialized.empty() || !initializedNotification.HasBeenNotified()) {
        return;
    }

    logger->debug("Releasing {} Watchman notification(s) held during initialization", heldUntilInitialized.size());
    absl::MutexLock lck(&messageQueueMutex);
    for (auto &msg : heldUntilInitialized) {
        msg->tagNewRequest(*logger);
        messageQueue.pendingRequests.push_back(move(msg));
    }
    heldUntilInitialized.clear();
    messageQueue.counters = mergeCounters(move(messageQueue.counters));
}

void WatchmanProcess::processQueryResponse(unique_ptr<WatchmanQueryResponse> response) {
    auto notifMsg = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(response));
    enqueueNotification(move(notifMsg));
}

void WatchmanProcess::processStateEnter(unique_ptr<sorbet::realmain::lsp::WatchmanStateEnter> stateEnter) {
    auto notification = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanStateEnter, move(stateEnter));
    enqueueNotification(move(notification));
}

void WatchmanProcess::processStateLeave(unique_ptr<sorbet::realmain::lsp::WatchmanStateLeave> stateLeave) {
    auto notification = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanStateLeave, move(stateLeave));
    enqueueNotification(move(notification));
}

void WatchmanProcess::processExit(int watchmanExitCode, const optional<string> &msg) {
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
