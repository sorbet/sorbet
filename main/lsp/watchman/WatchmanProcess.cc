#include "WatchmanProcess.h"
#include "subprocess.hpp"

using namespace std;

namespace sorbet::realmain::lsp::watchman {

WatchmanProcess::WatchmanProcess(
    shared_ptr<spdlog::logger> logger, string_view workSpace, vector<string> extensions,
    function<void(rapidjson::MemoryPoolAllocator<> &, const sorbet::realmain::lsp::WatchmanQueryResponse &)>
        processUpdate,
    std::function<void(rapidjson::MemoryPoolAllocator<> &, int)> processExit)
    : logger(logger), workSpace(string(workSpace)), extensions(extensions), processUpdate(processUpdate),
      processExit(processExit), thread(runInAThread("watchmanReader", std::bind(&WatchmanProcess::start, this))) {}

WatchmanProcess::~WatchmanProcess() {
    logger->debug("Ending watchman process.");
};

string getLineFromFd(int fd, FILE *file, string &buffer) {
    auto bufferFnd = buffer.find('\n');
    if (bufferFnd != string::npos) {
        // Edge case: Last time this was called, we read multiple messages off the line.
        string rv = buffer.substr(0, bufferFnd);
        buffer = buffer.substr(bufferFnd + 1);
        return rv;
    }

    constexpr int BUFF_SIZE = 1024 * 8;
    vector<char> buf(BUFF_SIZE);

    fcntl(fd, F_SETFL, (fcntl(fd, F_GETFL) | O_NONBLOCK));

    while (true) {
        auto bytesRead = read(fd, buf.data(), BUFF_SIZE);
        if (bytesRead <= 0) {
            if (errno == EAGAIN) {
                fcntl(fd, F_SETFL, (fcntl(fd, F_GETFL) & ~O_NONBLOCK));
                bytesRead = fread(buf.data(), 1, 1, file);

                fcntl(fd, F_SETFL, (fcntl(fd, F_GETFL) | O_NONBLOCK));
            }
        }
        if (bytesRead > 0) {
            auto fnd = std::find(buf.begin(), buf.begin() + bytesRead, '\n');
            if (fnd != buf.begin() + bytesRead) {
                buffer.append(buf.begin(), fnd);
                string rv = buffer;
                buffer.clear();
                // If we read beyond the line, store extra stuff we read into the string buffer.
                // Skip over the newline.
                buffer.append(fnd + 1, buf.begin() + bytesRead);
                return rv;
            } else {
                buffer.append(buf.begin(), buf.begin() + bytesRead);
            }
        }
    }
}

void WatchmanProcess::start() {
    try {
        string subscriptionName = fmt::format("ruby-typer-{}", getpid());

        logger->debug("Starting monitoring path {} with watchman for files with extensions {}. Subscription id: {}",
                      workSpace, fmt::join(extensions, ","), subscriptionName);

        auto p = subprocess::Popen({"watchman", "-j", "-p", "--no-pretty"}, subprocess::output{subprocess::PIPE},
                                   subprocess::input{subprocess::PIPE});

        // Note: Newer versions of Watchman (post 4.9.0) support ["suffix", ["suffix1", "suffix2", ...]], but Stripe
        // laptops have 4.9.0. Thus, we use [ "anyof", [ "suffix", "suffix1" ], [ "suffix", "suffix2" ], ... ].
        // Note 2: `empty_on_fresh_instance` prevents Watchman from sending entire contents of folder if this
        // subscription starts the daemon / causes the daemon to watch this folder for the first time.
        string subscribeCommand = fmt::format("[\"subscribe\", \"{}\", \"{}\", {{\n"
                                              "  \"expression\": [\"allof\", "
                                              "    [\"type\", \"f\"],\n"
                                              "    [\"anyof\", {}]"
                                              "  ],\n"
                                              "  \"defer_vcs\": false,\n"
                                              "  \"fields\": [\"name\"],\n"
                                              "  \"empty_on_fresh_instance\": true\n"
                                              "}}]\n",
                                              workSpace, subscriptionName,
                                              fmt::map_join(extensions, ",", [](const std::string &ext) -> string {
                                                  return fmt::format("[\"suffix\", \"{}\"]", ext);
                                              }));
        p.send(subscribeCommand.c_str(), subscribeCommand.size());
        logger->debug(subscribeCommand);

        auto file = p.output();
        auto fd = fileno(file);

        string buffer;

        while (true) {
            string line = getLineFromFd(fd, file, buffer);
            rapidjson::Document d(&alloc);
            logger->debug(line);
            if (d.Parse(line.c_str()).HasParseError()) {
                logger->error("Error parsing Watchman response: `{}` is not a valid json object", line);
            } else if (d.HasMember("is_fresh_instance")) {
                try {
                    auto queryResponse = sorbet::realmain::lsp::WatchmanQueryResponse::fromJSONValue(alloc, d);
                    processUpdate(alloc, *queryResponse);
                } catch (sorbet::realmain::lsp::DeserializationError e) {
                    // Gracefully handle deserialization errors, since they could be our fault.
                    logger->error("Unable to deserialize Watchman request: {}\nOriginal request:\n{}", e.what(), line);
                }
            } else if (!d.HasMember("subscribe")) {
                // Not a subscription response, or a file update.
                logger->debug("Unknown Watchman response:\n{}", line);
            }
        }
    } catch (std::exception e) {
        // Swallow error and print an informative message.
        logger->info("Error running Watchman: {}\nSorbet will not be able to detect changes to files made outside of "
                     "your code editor.\nDon't need Watchman? Run Sorbet with `--disable-watchman`.",
                     e.what());
        processExit(alloc, 1);
    }
}

} // namespace sorbet::realmain::lsp::watchman