#include "doctest.h"
// has to go first as it violates our requirements
#include "common/kvstore/KeyValueStore.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Loc.h"
#include "core/Unfreeze.h"
#include "main/lsp/ErrorReporter.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "payload/payload.h"
#include "spdlog/sinks/null_sink.h"
#include "test/helpers/MockFileSystem.h"
using namespace std;

namespace sorbet::realmain::lsp::test {
namespace {

options::Options makeOptions(string_view rootPath) {
    options::Options opts;
    opts.rawInputDirNames.emplace_back(string(rootPath));
    opts.runLSP = true;
    opts.fs = make_shared<sorbet::test::MockFileSystem>(rootPath);
    return opts;
}
auto nullSink = make_shared<spdlog::sinks::null_sink_mt>();
auto nullOpts = makeOptions("");
auto logger = make_shared<spdlog::logger>("console", nullSink);

shared_ptr<LSPConfiguration> makeConfig() {
    auto config = make_shared<LSPConfiguration>(nullOpts, make_shared<LSPOutputToVector>(), logger, true, false);
    InitializeParams initParams("", make_unique<ClientCapabilities>());
    initParams.rootPath = "";
    initParams.initializationOptions = make_unique<SorbetInitializationOptions>();
    config->setClientConfig(make_shared<LSPClientConfiguration>(initParams));
    return config;
}

unique_ptr<core::GlobalState> makeGS() {
    auto gs = make_unique<core::GlobalState>((make_shared<core::ErrorQueue>(*logger, *logger)));
    unique_ptr<const OwnedKeyValueStore> kvstore;
    payload::createInitialGlobalState(gs, nullOpts, kvstore);
    gs->errorQueue->ignoreFlushes = true;
    return gs;
}

} // namespace
TEST_CASE("NotifiesVSCodeWhenFileHasErrors") {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> errors;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 0;
        auto file = make_shared<core::File>("foo.rb", "foo", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);
        auto outputVector = dynamic_pointer_cast<LSPOutputToVector>(cs->output);

        errors.emplace_back(
            make_unique<core::Error>(core::Loc(fref, 0, 0), core::ErrorClass{1, core::StrictLevel::True}, "MyError",
                                     vector<core::ErrorSection>(), vector<core::AutocorrectSuggestion>(), false));
        er.pushDiagnostics(epoch, fref, errors, *gs);

        auto output = outputVector->getOutput();
        CHECK_EQ(1, output.size());

        auto &message = output.back();
        auto &notificationMessage = message->asNotification();
        auto &publishDiagnosticParams = get<unique_ptr<PublishDiagnosticsParams>>(notificationMessage.params);

        INFO("Reports file with errors to VS code");
        CHECK_EQ(publishDiagnosticParams->uri, cs->fileRef2Uri(*gs, fref));
        CHECK_EQ(1, publishDiagnosticParams->diagnostics.size());
    }
}

TEST_CASE("ReportsEmptyErrorsToVSCodeIfFilePreviouslyHadErrors") {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> emptyErrorList;
    vector<unique_ptr<core::Error>> errors;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 0;
        auto newEpoch = 1;
        auto file = make_shared<core::File>("foo.rb", "foo", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);
        auto outputVector = dynamic_pointer_cast<LSPOutputToVector>(cs->output);

        errors.emplace_back(
            make_unique<core::Error>(core::Loc(fref, 0, 0), core::ErrorClass{1, core::StrictLevel::True}, "MyError",
                                     vector<core::ErrorSection>(), vector<core::AutocorrectSuggestion>(), false));
        er.pushDiagnostics(epoch, fref, errors, *gs);

        er.pushDiagnostics(newEpoch, fref, emptyErrorList, *gs);
        auto output = outputVector->getOutput();
        CHECK_EQ(2, output.size());

        auto &latestMessage = output.back();
        auto &latestDiagnosticParams =
            get<unique_ptr<PublishDiagnosticsParams>>(latestMessage->asNotification().params);

        INFO("Sends empty errors to VS code when file previously had errors");
        CHECK_EQ(latestDiagnosticParams->uri, cs->fileRef2Uri(*gs, fref));
        CHECK(latestDiagnosticParams->diagnostics.empty());
    }
}

TEST_CASE("DoesNotReportToVSCodeWhenFileNeverHadErrors") {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> emptyErrorList;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 0;
        auto newEpoch = 1;
        auto file = make_shared<core::File>("foo.rb", "foo", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);

        auto outputVector = dynamic_pointer_cast<LSPOutputToVector>(cs->output);

        er.pushDiagnostics(epoch, fref, emptyErrorList, *gs);
        er.pushDiagnostics(newEpoch, fref, emptyErrorList, *gs);

        auto output = outputVector->getOutput();
        CHECK(output.empty());
    }
}

TEST_CASE("ErrorReporterIgnoresErrorsFromOldEpochs") {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> emptyErrorList;
    vector<unique_ptr<core::Error>> errors;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto initialEpoch = 0;
        auto slowPathEpoch = 1;
        auto latestEpoch = 2;
        auto file = make_shared<core::File>("foo.rb", "foo", core::File::Type::Normal, initialEpoch);
        auto fref = gs->enterFile(file);
        auto outputVector = dynamic_pointer_cast<LSPOutputToVector>(cs->output);
        errors.emplace_back(
            make_unique<core::Error>(core::Loc(fref, 0, 0), core::ErrorClass{1, core::StrictLevel::True}, "MyError",
                                     vector<core::ErrorSection>(), vector<core::AutocorrectSuggestion>(), false));

        // pushDiagnostics is called for foo.rb at epoch 0 with no errors (initial state)
        er.pushDiagnostics(initialEpoch, fref, emptyErrorList, *gs);
        // pushDiagnostics is called for foo.rb at epoch 2 with no errors (the fast path preemption)
        er.pushDiagnostics(latestEpoch, fref, emptyErrorList, *gs);
        // pushDiagnostics is called for foo.rb at epoch 1 with errors (the slow path)
        er.pushDiagnostics(slowPathEpoch, fref, errors, *gs);

        // We expect that no errors are reported to VS Code
        auto output = outputVector->getOutput();
        CHECK(output.empty());
    }
}
TEST_CASE("filesWithErrorsSince") {
    auto cs = makeConfig();
    auto gs = makeGS();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> errors;
    vector<unique_ptr<core::Error>> emptyErrorList;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 0;
        auto requestedEpoch = 3;
        auto file = make_shared<core::File>("foo.rb", "foo", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);
        auto fileWithoutErrors = make_shared<core::File>("bar.rb", "bar", core::File::Type::Normal, epoch);
        auto frefWithoutErrors = gs->enterFile(fileWithoutErrors);
        errors.emplace_back(
            make_unique<core::Error>(core::Loc(fref, 0, 0), core::ErrorClass{1, core::StrictLevel::True}, "MyError",
                                     vector<core::ErrorSection>(), vector<core::AutocorrectSuggestion>(), false));

        er.pushDiagnostics(epoch, fref, errors, *gs);
        INFO("Only returns files with lastReportedEpoch >= sent epoch");
        CHECK(er.filesWithErrorsSince(requestedEpoch).empty());

        er.pushDiagnostics(requestedEpoch, fref, errors, *gs);
        er.pushDiagnostics(requestedEpoch, frefWithoutErrors, emptyErrorList, *gs);

        auto filesWithErrorsSince = er.filesWithErrorsSince(requestedEpoch);
        INFO("Only returns files with errors");
        CHECK_EQ(1, filesWithErrorsSince.size());
        CHECK_EQ(fref, filesWithErrorsSince[0]);
    }
}
} // namespace sorbet::realmain::lsp::test
