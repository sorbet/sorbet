#include "WatchmanProcess.h"
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

struct ShutdownGuard {
    subprocess::Popen &p;
    ~ShutdownGuard() noexcept {
        shutdownWatchmanChild(p);
    }
};

// Longer than the 60s watchman spends synchronizing with the file system before it gives up on a query.
constexpr auto CHANGES_SINCE_QUERY_TIMEOUT = chrono::seconds(90);

} // namespace

string WatchmanProcess::resolveWatchRoot() {
    if (watchmanNamespace.empty()) {
        return workSpace;
    }

    const optional<string> maybeResolved = FileOps::realpath(workSpace);
    if (!maybeResolved.has_value()) {
        logger->debug("Unable to resolve workspace path {} for namespacing", workSpace);
        watchmanNamespace.clear();
        return workSpace;
    }

    string_view root(*maybeResolved);
    logger->debug("realpath({}) = {}", workSpace, root);
    if (!absl::ConsumeSuffix(&root, watchmanNamespace)) {
        logger->debug("Watched directory {} is not in namespace {}, disabling namespacing", root, watchmanNamespace);
        watchmanNamespace.clear();
        return workSpace;
    }

    string gitDirectory(root);
    gitDirectory += "/.git";
    if (!FileOps::dirExists(gitDirectory)) {
        logger->debug("Parent directory {} of namespace {} is not a git repository, disabling namespacing", root,
                      watchmanNamespace);
        watchmanNamespace.clear();
        return workSpace;
    }

    logger->debug("Using {} as watchman root", root);
    return string(root);
}

void WatchmanProcess::start() {
    auto mainPid = getpid();
    try {
        string subscriptionName = fmt::format("ruby-typer-{}", getpid());
        string watchRoot = resolveWatchRoot();

        // Counts only children that died before watchman acknowledged their subscription.
        int failedRestarts = 0;
        while (!isStopped()) {
            auto run = runSubscription(watchRoot, subscriptionName);
            if (run.end == SubscriptionEnd::Stopped) {
                break;
            }

            if (run.end == SubscriptionEnd::DeltaLost) {
                auto msg =
                    fmt::format("Watchman restarted while Sorbet was running, so Sorbet no longer knows which "
                                "files changed on disk. Restart Sorbet to pick up the current file system state.");
                logger->error(msg);
                exitWithCode(1, msg);
                break;
            }

            ENFORCE(run.end == SubscriptionEnd::ChildExited);
            failedRestarts = run.subscribed ? 1 : failedRestarts + 1;
            if (failedRestarts > MAX_WATCHMAN_RESTARTS) {
                auto msg = fmt::format("The Watchman CLI (`{} -j -p --no-pretty`) exited {} times in a row without "
                                       "delivering a subscription. Sorbet cannot detect changes to files made outside "
                                       "of your code editor without it, so it is exiting.",
                                       watchmanPath, failedRestarts);
                logger->error(msg);
                exitWithCode(1, msg);
                break;
            }

            auto delay = watchmanRestartDelay(failedRestarts);
            logger->error("The Watchman CLI exited unexpectedly. Restarting it in {}ms and resubscribing (attempt {} "
                          "of {}); the changes since clock {} will be fetched once it is up",
                          delay.count(), failedRestarts, MAX_WATCHMAN_RESTARTS, lastClock.value_or("<none>"));
            sleepUnlessStopped(delay);
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

WatchmanProcess::SubscriptionRun WatchmanProcess::runSubscription(const string &watchRoot,
                                                                  const string &subscriptionName) {
    auto p = subprocess::Popen({watchmanPath.c_str(), "-j", "-p", "--no-pretty"}, subprocess::output{subprocess::PIPE},
                               subprocess::input{subprocess::PIPE});

    // Declared after `p` so it only runs if construction succeeded. Runs on every exit path
    // out of this scope — normal return, break, or exception — so the watchman child is
    // always reaped before this thread returns.
    ShutdownGuard shutdownGuard{p};

    // Fetched by query rather than by giving the subscription a `since`: watchman runs a subscription's initial
    // results with sync_timeout 0, so they can miss a change it was still digesting at subscribe time.
    const optional<string> sinceClock = lastClock;

    logger->debug("Starting monitoring path {} with watchman for files with extensions {}. Subscription id: {}",
                  watchRoot, fmt::join(extensions, ","), subscriptionName);

    string subscribeCommand = buildSubscribeCommand(watchRoot, subscriptionName, extensions, watchmanNamespace);
    p.send(subscribeCommand.c_str(), subscribeCommand.size());
    logger->debug(subscribeCommand);

    auto file = p.output();
    auto fd = fileno(file);

    string buffer;
    SubscriptionRun run{SubscriptionEnd::ChildExited, false};

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

            logger->error("Watchman CLI stdout closed (errno={})", errno);
            return run;
        }

        ENFORCE(maybeLine.result == FileOps::ReadResult::Success);

        const string &line = *maybeLine.output;
        // Line found!
        rapidjson::MemoryPoolAllocator<> alloc;
        rapidjson::Document d(&alloc);
        logger->debug(line);
        if (d.Parse(line.c_str(), line.size()).HasParseError()) {
            logger->error("Error parsing Watchman response: `{}` is not a valid json object", line);
        } else if (d.HasMember("error")) {
            // Subscribing is the only command we send, so an error leaves this child with nothing to deliver.
            logger->error("Watchman returned an error: {}", line);
            return run;
        } else if (d.HasMember("canceled")) {
            // `watchman watch-del` drops the watch behind the subscription; nothing further arrives on this child.
            logger->error("Watchman canceled our subscription: {}", line);
            return run;
        } else if (d.HasMember("is_fresh_instance")) {
            catchDeserializationError(*logger, line, [&d, this]() {
                auto queryResponse = sorbet::realmain::lsp::WatchmanQueryResponse::fromJSONValue(d);
                lastClock = queryResponse->clock;
                stripNamespace(*queryResponse);
                processQueryResponse(move(queryResponse));
            });
        } else if (d.HasMember("state-enter")) {
            // These are messages from "state-enter" commands.  See
            // https://facebook.github.io/watchman/docs/cmd/state-enter.html
            // for more information.
            catchDeserializationError(*logger, line, [&d, this]() {
                auto stateEnter = sorbet::realmain::lsp::WatchmanStateEnter::fromJSONValue(d);
                lastClock = stateEnter->clock;
                processStateEnter(move(stateEnter));
            });
        } else if (d.HasMember("state-leave")) {
            // These are messages from "state-leave" commands.  See
            // https://facebook.github.io/watchman/docs/cmd/state-leave.html
            // for more information.
            catchDeserializationError(*logger, line, [&d, this]() {
                auto stateLeave = sorbet::realmain::lsp::WatchmanStateLeave::fromJSONValue(d);
                lastClock = stateLeave->clock;
                processStateLeave(move(stateLeave));
            });
        } else if (d.HasMember("subscribe")) {
            run.subscribed = true;
            if (d.HasMember("clock") && d["clock"].IsString()) {
                lastClock = string(d["clock"].GetString(), d["clock"].GetStringLength());
            }

            if (sinceClock.has_value()) {
                if (auto failure = fetchChangesSince(watchRoot, *sinceClock)) {
                    if (*failure == SubscriptionEnd::ChildExited) {
                        // Rewind so the next attempt asks for a superset of the delta this one failed to fetch.
                        lastClock = sinceClock;
                    }
                    run.end = *failure;
                    return run;
                }
            }
        } else {
            // Something we don't understand yet.
            logger->debug("Unknown Watchman response:\n{}", line);
        }
    }

    run.end = SubscriptionEnd::Stopped;
    return run;
}

optional<WatchmanProcess::SubscriptionEnd> WatchmanProcess::fetchChangesSince(const string &watchRoot,
                                                                              const string &since) {
    auto p = subprocess::Popen({watchmanPath.c_str(), "-j", "--no-pretty"}, subprocess::output{subprocess::PIPE},
                               subprocess::input{subprocess::PIPE});
    ShutdownGuard shutdownGuard{p};

    string query = buildChangesSinceQuery(watchRoot, extensions, watchmanNamespace, since);
    logger->debug("Fetching the files that changed while the Watchman CLI was down: {}", query);
    p.send(query.c_str(), query.size());
    p.close_input();

    auto fd = fileno(p.output());
    string buffer;
    auto deadline = chrono::steady_clock::now() + CHANGES_SINCE_QUERY_TIMEOUT;
    while (!isStopped()) {
        if (chrono::steady_clock::now() > deadline) {
            logger->error("Watchman did not answer the changes-since query within {}s",
                          chrono::duration_cast<chrono::seconds>(CHANGES_SINCE_QUERY_TIMEOUT).count());
            return SubscriptionEnd::ChildExited;
        }

        errno = 0;
        auto maybeLine = FileOps::readLineFromFd(fd, buffer);
        if (maybeLine.result == FileOps::ReadResult::Timeout) {
            continue;
        }

        if (maybeLine.result == FileOps::ReadResult::ErrorOrEof) {
            if (errno == EINTR) {
                continue;
            }
            logger->error("Watchman exited without answering the changes-since query (errno={})", errno);
            return SubscriptionEnd::ChildExited;
        }

        const string &line = *maybeLine.output;
        logger->debug(line);
        rapidjson::MemoryPoolAllocator<> alloc;
        rapidjson::Document d(&alloc);
        if (d.Parse(line.c_str(), line.size()).HasParseError()) {
            logger->error("Error parsing Watchman response: `{}` is not a valid json object", line);
            return SubscriptionEnd::ChildExited;
        }

        if (d.HasMember("error") || !d.HasMember("is_fresh_instance")) {
            logger->error("Watchman rejected the changes-since query: {}", line);
            return SubscriptionEnd::ChildExited;
        }

        optional<SubscriptionEnd> failure = SubscriptionEnd::ChildExited;
        catchDeserializationError(*logger, line, [&d, &failure, this]() {
            auto queryResponse = sorbet::realmain::lsp::WatchmanQueryResponse::fromJSONValue(d);
            if (queryResponse->isFreshInstance) {
                // A clock from an earlier watchman instance, so what changed in between is unknowable.
                failure = SubscriptionEnd::DeltaLost;
                return;
            }

            logger->debug("Watchman reports {} files changed while its CLI was down", queryResponse->files.size());
            lastClock = queryResponse->clock;
            stripNamespace(*queryResponse);
            processQueryResponse(move(queryResponse));
            failure = nullopt;
        });
        return failure;
    }

    return SubscriptionEnd::ChildExited;
}

void WatchmanProcess::stripNamespace(WatchmanQueryResponse &response) const {
    if (watchmanNamespace.empty()) {
        return;
    }

    auto prefix(watchmanNamespace);
    prefix += "/";

    for (auto &file : response.files) {
        string_view view(file);
        if (!absl::ConsumePrefix(&view, prefix)) {
            continue;
        }
        file = view;
    }
}

void WatchmanProcess::sleepUnlessStopped(chrono::milliseconds delay) {
    absl::MutexLock lck(&mutex);
    mutex.AwaitWithTimeout(absl::Condition(&stopped), absl::Milliseconds(delay.count()));
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
    // Waiting here for the LSP loop to initialize would stop this thread draining the CLI's stdout, and watchman
    // drops a client that has not accepted its output for 60s, mid-message.
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

    logger->debug("Releasing {} Watchman notifications held during initialization", heldUntilInitialized.size());
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
