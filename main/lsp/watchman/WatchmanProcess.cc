#include "WatchmanProcess.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "common/strings/formatting.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "subprocess.hpp"
#include <chrono>
#include <thread>

using namespace std;

namespace sorbet::realmain::lsp::watchman {

WatchmanProcess::WatchmanProcess(shared_ptr<spdlog::logger> logger, string_view watchmanPath, string_view workSpace,
                                 vector<string> extensions, MessageQueueState &messageQueue,
                                 absl::Mutex &messageQueueMutex, absl::Notification &initializedNotification,
                                 shared_ptr<const LSPConfiguration> config)
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

struct WatchProjectResult {
    string watchRoot;
    // Path of the queried directory relative to watchRoot. Empty when the queried directory IS
    // the watch root.
    string relativePath;
};

// Synchronously read one line of JSON from watchman, polling isStopped() so the destructor can
// break us out of the wait. Returns nullopt iff isStopped() became true before a line arrived.
optional<string> readJsonLine(int fd, string &buffer, const std::function<bool()> &isStopped) {
    while (!isStopped()) {
        errno = 0;
        auto r = sorbet::FileOps::readLineFromFd(fd, buffer);
        if (r.result == sorbet::FileOps::ReadResult::Timeout) {
            continue;
        }
        if (r.result == sorbet::FileOps::ReadResult::ErrorOrEof) {
            if (errno == EINTR) {
                continue;
            }
            throw runtime_error("watchman closed pipe before responding");
        }
        ENFORCE(r.result == sorbet::FileOps::ReadResult::Success);
        return std::move(*r.output);
    }
    return nullopt;
}

// Issues `["watch-project", workspace]` to watchman and parses the response. Sorbet's workspace
// can sit inside an existing watched root (e.g. a subdirectory of a larger repo whose top level
// is already watched). Watchman rejects subscribes whose root isn't a registered watch, so we
// ask it to resolve the workspace to the right (watch, relative_path) pair via watch-project.
// https://facebook.github.io/watchman/docs/cmd/watch-project.html
WatchProjectResult resolveWatchProject(subprocess::Popen &p, int fd, string &buffer, string_view workspace,
                                       spdlog::logger &logger, const std::function<bool()> &isStopped) {
    rapidjson::StringBuffer buf;
    rapidjson::Writer<rapidjson::StringBuffer> w(buf);
    w.StartArray();
    w.String("watch-project");
    w.String(workspace.data(), workspace.size());
    w.EndArray();

    string cmd = buf.GetString();
    p.send(cmd.c_str(), cmd.size());
    logger.debug(cmd);

    auto line = readJsonLine(fd, buffer, isStopped);
    if (!line.has_value()) {
        throw runtime_error("watchman watch-project interrupted by shutdown");
    }
    logger.debug(*line);

    rapidjson::MemoryPoolAllocator<> alloc;
    rapidjson::Document doc(&alloc);
    if (doc.Parse(line->c_str(), line->size()).HasParseError() || !doc.IsObject()) {
        throw runtime_error(fmt::format("watch-project response was not a JSON object: {}", *line));
    }
    if (doc.HasMember("error") && doc["error"].IsString()) {
        throw runtime_error(fmt::format("watch-project failed: {}", doc["error"].GetString()));
    }
    if (!doc.HasMember("watch") || !doc["watch"].IsString()) {
        throw runtime_error(fmt::format("watch-project response missing 'watch' field: {}", *line));
    }

    WatchProjectResult result;
    result.watchRoot = doc["watch"].GetString();
    if (doc.HasMember("relative_path") && doc["relative_path"].IsString()) {
        result.relativePath = doc["relative_path"].GetString();
    }
    return result;
}

// Attempt to shut down the watchman CLI cleanly. The CLI's `-p` (persistent) mode keeps a unix
// socket connection to the watchman daemon open until its stdin closes AND its parent has gone
// away. If we just let Popen go out of scope we close the FDs, but cpp-subprocess never reaps —
// the daemon-connection socket can outlive sorbet, leaving the daemon to grow thread/port
// counts unboundedly across editor restarts. So: close stdin, give it a beat to exit on EOF,
// then SIGTERM if it's still around, and reap.
void shutdownWatchmanChild(subprocess::Popen &p, spdlog::logger &logger) {
    try {
        p.close_input();
    } catch (...) {
        // best-effort
    }
    for (int i = 0; i < 10; i++) {
        try {
            if (p.poll() != -1) {
                return;
            }
        } catch (...) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    try {
        p.kill(SIGTERM);
    } catch (...) {
        // best-effort
    }
    try {
        p.wait();
    } catch (...) {
        // best-effort
    }
}

} // namespace

void WatchmanProcess::start() {
    auto mainPid = getpid();
    try {
        string subscriptionName = fmt::format("ruby-typer-{}", getpid());

        auto p = subprocess::Popen({watchmanPath.c_str(), "-j", "-p", "--no-pretty"},
                                   subprocess::output{subprocess::PIPE}, subprocess::input{subprocess::PIPE});

        // Declared after `p` so it only runs if construction succeeded. Runs on every exit path
        // out of this scope — normal return, break, or exception — so the watchman child is
        // always reaped before this thread returns.
        struct ShutdownGuard {
            subprocess::Popen &p;
            spdlog::logger &logger;
            ~ShutdownGuard() noexcept {
                shutdownWatchmanChild(p, logger);
            }
        } shutdownGuard{p, *logger};

        auto file = p.output();
        auto fd = fileno(file);
        string buffer;

        auto stoppedFn = [this]() { return this->isStopped(); };

        // Resolve workspace → (watch root, relative path) so the subscribe targets a registered
        // watchman root rather than an arbitrary subdirectory.
        WatchProjectResult resolved = resolveWatchProject(p, fd, buffer, workSpace, *logger, stoppedFn);

        logger->debug("Starting monitoring path {} (watch root {}, relative_root {}) with watchman for files with "
                      "extensions {}. Subscription id: {}",
                      workSpace, resolved.watchRoot, resolved.relativePath.empty() ? "<none>" : resolved.relativePath,
                      fmt::join(extensions, ","), subscriptionName);

        rapidjson::StringBuffer subscribeCommandBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> w(subscribeCommandBuffer);
        {
            w.StartArray();
            w.String("subscribe");
            w.String(resolved.watchRoot);
            w.String(subscriptionName);

            {
                w.StartObject();

                w.String("expression");
                {
                    w.StartArray();
                    w.String("allof");
                    {
                        w.StartArray();
                        w.String("type");
                        w.String("f");
                        w.EndArray();
                    }

                    // Note: Newer versions of Watchman (post 4.9.0) support ["suffix", ["suffix1", "suffix2", ...]],
                    // but Stripe laptops have 4.9.0. Thus, we use [ "anyof", [ "suffix", "suffix1" ], [ "suffix",
                    // "suffix2" ], ... ].
                    {
                        w.StartArray();
                        w.String("anyof");

                        for (auto &extension : extensions) {
                            w.StartArray();
                            w.String("suffix");
                            w.String(extension);
                            w.EndArray();
                        }

                        w.EndArray();
                    }

                    // Exclude rsync tmpfiles
                    {
                        w.StartArray();
                        w.String("not");
                        {
                            w.StartArray();
                            w.String("match");
                            w.String("**/.~tmp~/**");
                            w.String("wholename");
                            {
                                w.StartObject();
                                w.String("includedotfiles");
                                w.Bool(true);
                                w.EndObject();
                            }
                            w.EndArray();
                        }
                        w.EndArray();
                    }

                    w.EndArray();
                }

                w.String("fields");
                {
                    w.StartArray();
                    w.String("name");
                    w.EndArray();
                }

                // When workspace is a subdirectory of the watch root, scope the subscription to it.
                // Watchman returns file names relative to relative_root, matching the contract the
                // rest of Sorbet expects. https://facebook.github.io/watchman/docs/cmd/subscribe.html
                if (!resolved.relativePath.empty()) {
                    w.String("relative_root");
                    w.String(resolved.relativePath);
                }

                // Note 2: `empty_on_fresh_instance` prevents Watchman from sending entire contents of folder if this
                // subscription starts the daemon / causes the daemon to watch this folder for the first time.
                w.String("empty_on_fresh_instance");
                w.Bool(true);

                w.EndObject();
            }

            w.EndArray();
        }

        string subscribeCommand = subscribeCommandBuffer.GetString();
        p.send(subscribeCommand.c_str(), subscribeCommand.size());
        logger->debug(subscribeCommand);

        while (!isStopped()) {
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

                // Exit loop; unable to read from Watchman process.
                exitWithCode(1, nullopt);
                break;
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
            } else if (!d.HasMember("subscribe")) {
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

void WatchmanProcess::exitWithCode(int code, const optional<string> &msg) {
    absl::MutexLock lck(&mutex);
    if (!stopped) {
        stopped = true;
        processExit(code, msg);
    }
}

void WatchmanProcess::enqueueNotification(unique_ptr<NotificationMessage> notification) {
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
