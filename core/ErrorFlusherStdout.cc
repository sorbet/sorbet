#include "core/ErrorFlusherStdout.h"
#include "common/FileSystem.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet::core {

void ErrorFlusherStdout::flushErrors(spdlog::logger &logger, const GlobalState &gs, core::FileRef file,
                                     vector<unique_ptr<ErrorQueueMessage>> errors) {
    fmt::memory_buffer critical, nonCritical;
    for (auto &error : errors) {
        if (error->kind == ErrorQueueMessage::Kind::Error) {
            if (error->error->isSilenced) {
                continue;
            }

            prodHistogramAdd("error", error->error->what.code, 1);

            auto &out = error->error->isCritical() ? critical : nonCritical;
            if (out.size() != 0) {
                fmt::format_to(out, "\n\n");
            }
            ENFORCE(error->text.has_value());
            fmt::format_to(out, "{}", error->text.value_or(""));

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

void ErrorFlusherStdout::flushAutocorrects(const GlobalState &gs, FileSystem &fs) {
    UnorderedMap<FileRef, string> sources;
    for (auto &autocorrect : autocorrects) {
        for (auto &edit : autocorrect.edits) {
            auto file = edit.loc.file();
            if (!sources.count(file)) {
                sources[file] = fs.readFile(file.data(gs).path());
            }
        }
    }

    auto toWrite = AutocorrectSuggestion::apply(autocorrects, sources);
    for (auto &entry : toWrite) {
        fs.writeFile(entry.first.data(gs).path(), entry.second);
    }
    autocorrects.clear();
}

void ErrorFlusherStdout::flushErrorCount(spdlog::logger &logger, int count) {
    if (count == 0) {
        logger.log(spdlog::level::err, "No errors! Great job.", count);
    } else {
        logger.log(spdlog::level::err, "Errors: {}", count);
    }
}

} // namespace sorbet::core
