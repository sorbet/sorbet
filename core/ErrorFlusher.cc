#include "core/ErrorFlusher.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet::core {

void ErrorFlusher::flushErrors(spdlog::logger &logger, vector<unique_ptr<ErrorQueueMessage>> errors) {
    fmt::memory_buffer critical, nonCritical;
    for (auto &error : errors) {
        if (error->kind == ErrorQueueMessage::Kind::Error) {
            if (error->error->isSilenced) {
                continue;
            }
            auto &out = error->error->isCritical ? critical : nonCritical;
            if (out.size() != 0) {
                fmt::format_to(out, "\n\n");
            }
            fmt::format_to(out, "{}", error->text);

            for (auto &autocorrect : error->error->autocorrects) {
                autocorrects.emplace_back(move(autocorrect));
            }
        }
    }

    if (critical.size() != 0) {
        if (!printedAtLeastOneError) {
            logger.log(spdlog::level::critical, "{}", to_string(critical));
            printedAtLeastOneError = true;
        } else {
            logger.log(spdlog::level::critical, "\n{}", to_string(critical));
        }
    }
    if (nonCritical.size() != 0) {
        if (!printedAtLeastOneError) {
            logger.log(spdlog::level::err, "{}", to_string(nonCritical));
            printedAtLeastOneError = true;
        } else {
            logger.log(spdlog::level::err, "\n{}", to_string(nonCritical));
        }
    }
}

void ErrorFlusher::flushErrorCount(spdlog::logger &logger, int count) {
    if (count == 0) {
        logger.log(spdlog::level::err, "No errors! Great job.", count);
    } else {
        logger.log(spdlog::level::err, "Errors: {}", count);
    }
}

void ErrorFlusher::flushAutocorrects(const GlobalState &gs) {
    UnorderedMap<FileRef, string> sources;
    for (auto &autocorrect : autocorrects) {
        auto file = autocorrect.loc.file();
        if (!sources.count(file)) {
            sources[file] = FileOps::read(file.data(gs).path());
        }
    }

    auto toWrite = AutocorrectSuggestion::apply(autocorrects, sources);
    for (auto &entry : toWrite) {
        FileOps::write(entry.first.data(gs).path(), entry.second);
    }
    autocorrects.clear();
}

} // namespace sorbet::core
