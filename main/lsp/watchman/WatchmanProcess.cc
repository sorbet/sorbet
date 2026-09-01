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

#include <algorithm>
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

// Reaps a watchman CLI child on every exit path out of the scope that declared it.
struct ShutdownGuard {
    subprocess::Popen &p;
    ~ShutdownGuard() noexcept {
        shutdownWatchmanChild(p);
    }
};

optional<string> clockOf(const rapidjson::Document &d) {
    if (d.HasMember("clock") && d["clock"].IsString()) {
        return string(d["clock"].GetString(), d["clock"].GetStringLength());
    }
    return nullopt;
}

// How long a one-shot `watchman -j` query may take. Watchman's own file system synchronization gives up after 60s.
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

        // Consecutive children that died without watchman ever acknowledging their subscription.
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

    // Set when a previous child delivered responses: the changes made since then have to be fetched separately,
    // because they were never delivered to us. A subscription's own `since` would not do: its initial results are
    // computed without synchronizing with the file system, so they can miss a change watchman was still digesting.
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
        // The LSP loop may have finished initializing while we were waiting on watchman.
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

            // Unable to read from the Watchman process: it exited, or its connection to the daemon was severed
            // and it exited on the resulting decode error.
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
            // Watchman rejected a command. The only command we send is the subscribe, so without a subscription
            // this child is useless; the caller decides whether to try again.
            logger->error("Watchman returned an error: {}", line);
            return run;
        } else if (d.HasMember("canceled")) {
            // The watch behind our subscription was deleted (`watchman watch-del`). Nothing further arrives on this
            // child, so treat it like an exit and let the caller resubscribe.
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
            // The acknowledgement of our subscribe command. Its clock is the point from which the subscription
            // reports changes.
            run.subscribed = true;
            if (auto clock = clockOf(d)) {
                lastClock = *clock;
            }

            if (sinceClock.has_value()) {
                switch (fetchChangesSince(watchRoot, *sinceClock)) {
                    case DeltaFetch::Delivered:
                        break;
                    case DeltaFetch::Lost:
                        run.end = SubscriptionEnd::DeltaLost;
                        return run;
                    case DeltaFetch::Failed:
                        // Roll `lastClock` back so that the next attempt asks for the changes since the previous
                        // child's last clock again, a superset of the delta this attempt failed to fetch.
                        lastClock = sinceClock;
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

WatchmanProcess::DeltaFetch WatchmanProcess::fetchChangesSince(const string &watchRoot, const string &since) {
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
            return DeltaFetch::Failed;
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
            return DeltaFetch::Failed;
        }

        const string &line = *maybeLine.output;
        logger->debug(line);
        rapidjson::MemoryPoolAllocator<> alloc;
        rapidjson::Document d(&alloc);
        if (d.Parse(line.c_str(), line.size()).HasParseError()) {
            logger->error("Error parsing Watchman response: `{}` is not a valid json object", line);
            return DeltaFetch::Failed;
        }

        if (d.HasMember("error") || !d.HasMember("is_fresh_instance")) {
            logger->error("Watchman rejected the changes-since query: {}", line);
            return DeltaFetch::Failed;
        }

        auto result = DeltaFetch::Failed;
        catchDeserializationError(*logger, line, [&d, &result, this]() {
            auto queryResponse = sorbet::realmain::lsp::WatchmanQueryResponse::fromJSONValue(d);
            if (queryResponse->isFreshInstance) {
                // The clock belongs to another watchman instance (the daemon restarted, or the watch was deleted
                // and re-created), so nothing is known about what changed in between.
                result = DeltaFetch::Lost;
                return;
            }

            logger->debug("Watchman reports {} files changed while its CLI was down", queryResponse->files.size());
            lastClock = queryResponse->clock;
            stripNamespace(*queryResponse);
            processQueryResponse(move(queryResponse));
            result = DeltaFetch::Delivered;
        });
        return result;
    }

    return DeltaFetch::Failed;
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
    constexpr auto step = chrono::milliseconds(100);
    auto remaining = delay;
    while (remaining.count() > 0 && !isStopped()) {
        auto nap = min(step, remaining);
        this_thread::sleep_for(nap);
        remaining -= nap;
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
    // The preprocessor rejects file updates that arrive before the LSP loop has initialized (which includes the
    // initial typecheck). Hold them instead of blocking: a blocked reader stops draining the watchman CLI's stdout,
    // the CLI then stops reading from the watchman daemon, and the daemon drops a client that has not accepted its
    // output for 60s — mid-message, which kills the CLI once it resumes.
    if (!initializedNotification.HasBeenNotified()) {
        heldUntilInitialized.push_back(move(msg));
        return;
    }

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
