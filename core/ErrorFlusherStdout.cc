#include "core/ErrorFlusherStdout.h"
#include "common/FileSystem.h"
#include "core/GlobalState.h"
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
                fmt::format_to(std::back_inserter(out), "\n\n");
            }
            ENFORCE(error->text.has_value());
            fmt::format_to(std::back_inserter(out), "{}", error->text.value_or(""));

            for (auto &autocorrect : error->error->autocorrects) {
                if (autocorrect.skipWhenAggregated) {
                    continue;
                }
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

void ErrorFlusherStdout::flushErrorCount(spdlog::logger &logger, int count) {
    if (count == 0) {
        logger.log(spdlog::level::err, "No errors! Great job.", count);
    } else {
        logger.log(spdlog::level::err, "Errors: {}", count);
    }
}

void ErrorFlusherStdout::flushAutocorrects(const GlobalState &gs, FileSystem &fs) {
    if (gs.packageDB().enabled()) {
        for (auto &pkgName : gs.packageDB().packages()) {
            auto &pkg = gs.packageDB().getPackageInfo(pkgName);
            ENFORCE(pkg.exists());
            auto autocorrect = pkg.aggregateMissingImports();
            autocorrects.emplace_back(move(autocorrect));
        }
    }
    auto toWrite = AutocorrectSuggestion::apply(gs, fs, autocorrects);
    for (auto &[file, contents] : toWrite) {
        fs.writeFile(string(file.data(gs).path()), contents);
    }
    autocorrects.clear();
}

} // namespace sorbet::core
