#include "core/ErrorFlusher.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet {
namespace core {

void ErrorFlusher::flushErrors(spdlog::logger &logger, vector<unique_ptr<ErrorQueueMessage>> errors) {
    stringstream critical;
    stringstream nonCritical;
    for (auto &error : errors) {
        if (error->kind == ErrorQueueMessage::Kind::Error) {
            auto &out = error->error->isCritical ? critical : nonCritical;
            if (out.tellp() != 0) {
                out << '\n';
            }
            out << error->text;

            for (auto &autocorrect : error->error->autocorrects) {
                autocorrects.emplace_back(move(autocorrect));
            }
        }
    }

    if (critical.tellp() != 0) {
        if (!printedAtLeastOneError) {
            logger.log(spdlog::level::critical, "{}", critical.str());
            printedAtLeastOneError = true;
        } else {
            logger.log(spdlog::level::critical, "\n{}", critical.str());
        }
    }
    if (nonCritical.tellp() != 0) {
        if (!printedAtLeastOneError) {
            logger.log(spdlog::level::err, "{}", nonCritical.str());
            printedAtLeastOneError = true;
        } else {
            logger.log(spdlog::level::err, "\n{}", nonCritical.str());
        }
    }
}

void ErrorFlusher::flushErrorCount(spdlog::logger &logger, int count) {
    if (count == 0) {
        logger.log(spdlog::level::err, "No errors! Great job.\n", count);
    } else {
        logger.log(spdlog::level::err, "Errors: {}\n", count);
    }
}

void ErrorFlusher::flushAutocorrects(const GlobalState &gs) {
    map<FileRef, string> sources;
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

} // namespace core
} // namespace sorbet
