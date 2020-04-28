#include "gtest/gtest.h"
// has to go first as it violates our requirements
#include "common/kvstore/KeyValueStore.h"
#include "core/Error.h"
#include "core/ErrorQueue.h"
#include "core/Loc.h"
#include "core/Unfreeze.h"
#include "main/lsp/ErrorReporter.h"
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
TEST(ErrorReporterTest, FirstTimeFileWithErrors) {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> errors;
    errors.emplace_back(make_unique<core::Error>(core::Loc::none(), core::ErrorClass{1, core::StrictLevel::True},
                                                 "MyError", vector<core::ErrorSection>(),
                                                 vector<core::AutocorrectSuggestion>(), false));
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 0;
        auto newEpoch = 1;
        auto file = make_shared<core::File>("foo/bar", "", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);

        EXPECT_TRUE(er.getUncommittedFileErrorStatuses().empty())
            << fmt::format("uncomittedFileErrorStatuses should be empty");

        er.pushDiagnostics(newEpoch, fref, errors, *gs);
        EXPECT_EQ(1, er.getUncommittedFileErrorStatuses().size())
            << fmt::format("uncomittedFileErrorStatuses size should equal max file id");

        ErrorStatus fileErrorStatus = er.getUncommittedFileErrorStatuses().find(fref)->second;
        EXPECT_EQ(newEpoch, fileErrorStatus.sentEpoch)
            << fmt::format("File is added to uncomittedFileErrorStatuses with correct epoch");
        EXPECT_TRUE(fileErrorStatus.hasErrors) << fmt::format("File is added with hasErrors set to false");
    }
}

TEST(ErrorReporterTest, FileStillHasErrors) {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> errors;
    errors.emplace_back(make_unique<core::Error>(core::Loc::none(), core::ErrorClass{1, core::StrictLevel::True},
                                                 "MyError", vector<core::ErrorSection>(),
                                                 vector<core::AutocorrectSuggestion>(), false));
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 0;
        auto newEpoch = 1;
        auto file = make_shared<core::File>("foo/bar", "", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);

        er.pushDiagnostics(epoch, fref, errors, *gs);
        errors.emplace_back(make_unique<core::Error>(core::Loc::none(), core::ErrorClass{1, core::StrictLevel::True},
                                                     "MyError", vector<core::ErrorSection>(),
                                                     vector<core::AutocorrectSuggestion>(), false));
        er.pushDiagnostics(newEpoch, fref, errors, *gs);

        ErrorStatus uncommittedFileErrorStatus = er.getUncommittedFileErrorStatuses().find(fref)->second;
        EXPECT_EQ(newEpoch, uncommittedFileErrorStatus.sentEpoch)
            << fmt::format("Adds file to uncommittedFileErrorStatuses with new epoch");
    }
}

TEST(ErrorReporterTest, FileNoLongerHasErrors) {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> emptyErrorList;
    vector<unique_ptr<core::Error>> errors;
    errors.emplace_back(make_unique<core::Error>(core::Loc::none(), core::ErrorClass{1, core::StrictLevel::True},
                                                 "MyError", vector<core::ErrorSection>(),
                                                 vector<core::AutocorrectSuggestion>(), false));
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 0;
        auto newEpoch = 1;
        auto file = make_shared<core::File>("foo/without_error", "", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);

        er.pushDiagnostics(epoch, fref, errors, *gs);
        er.commit();
        er.pushDiagnostics(newEpoch, fref, emptyErrorList, *gs);

        ErrorStatus uncommittedFileErrorStatus = er.getUncommittedFileErrorStatuses().find(fref)->second;
        EXPECT_EQ(newEpoch, uncommittedFileErrorStatus.sentEpoch)
            << fmt::format("Updates the epoch of a file that no longer has errors");
        EXPECT_FALSE(uncommittedFileErrorStatus.hasErrors) << fmt::format("File hasErrors should be false");
    }
}

TEST(ErrorReporterTest, EpochLessThanLastCheckedEpoch) {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> errors;
    errors.emplace_back(make_unique<core::Error>(core::Loc::none(), core::ErrorClass{1, core::StrictLevel::True},
                                                 "MyError", vector<core::ErrorSection>(),
                                                 vector<core::AutocorrectSuggestion>(), false));
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 1;
        auto newEpoch = 0;
        auto file = make_shared<core::File>("foo/bar", "", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);

        er.pushDiagnostics(epoch, fref, errors, *gs);
        er.commit();

        er.pushDiagnostics(newEpoch, fref, errors, *gs);
        EXPECT_TRUE(er.getUncommittedFileErrorStatuses().empty()) << fmt::format(
            "File should not be added to uncommittedFileErrorStatuses if new epoch is less than previous epoch");
    }
}

TEST(ErrorReporterTest, EpochLessThanGSFileEpoch) {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> errors;
    errors.emplace_back(make_unique<core::Error>(core::Loc::none(), core::ErrorClass{1, core::StrictLevel::True},
                                                 "MyError", vector<core::ErrorSection>(),
                                                 vector<core::AutocorrectSuggestion>(), false));
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 1;
        auto newEpoch = 0;
        auto file = make_shared<core::File>("foo/bar", "", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);
        er.pushDiagnostics(epoch, fref, errors, *gs);
        er.commit();

        er.pushDiagnostics(newEpoch, fref, errors, *gs);
        EXPECT_TRUE(er.getUncommittedFileErrorStatuses().empty()) << fmt::format(
            "File should not be added to uncommittedFileErrorStatuses if new epoch is less than file epoch");
    }
}

TEST(ErrorReporterTest, ReportsErrorsToVSCode) {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> errors;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 0;
        auto file = make_shared<core::File>("foo/bar", "foo", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);

        errors.emplace_back(
            make_unique<core::Error>(core::Loc(fref, 0, 0), core::ErrorClass{1, core::StrictLevel::True}, "MyError",
                                     vector<core::ErrorSection>(), vector<core::AutocorrectSuggestion>(), false));

        auto outputVector = dynamic_pointer_cast<LSPOutputToVector>(cs->output);

        er.pushDiagnostics(epoch, fref, errors, *gs);

        auto output = outputVector->getOutput();
        auto &message = output[0];
        auto &notificationMessage = message->asNotification();
        auto &publishDiagnosticParams = get<unique_ptr<PublishDiagnosticsParams>>(notificationMessage.params);
        EXPECT_EQ(1, output.size());
        EXPECT_EQ(publishDiagnosticParams->uri, cs->fileRef2Uri(*gs, fref))
            << fmt::format("Reports files with errors to VS code");
    }
}

TEST(ErrorReporterTest, DoesNotReportWhenNoErrors) {
    auto gs = makeGS();
    auto cs = makeConfig();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> errors;
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 0;
        auto newEpoch = 1;
        auto file = make_shared<core::File>("foo/bar", "foo", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);
        er.pushDiagnostics(epoch, fref, errors, *gs);
        er.commit();

        auto outputVector = dynamic_pointer_cast<LSPOutputToVector>(cs->output);
        er.pushDiagnostics(newEpoch, fref, errors, *gs);
        auto output = outputVector->getOutput();
        EXPECT_EQ(0, output.size()) << fmt::format("Does not report files that never had errors to VS Code");
    }
}

TEST(ErrorReporterTest, Commit) {
    auto cs = makeConfig();
    auto gs = makeGS();
    ErrorReporter er(cs);
    vector<unique_ptr<core::Error>> errors;
    errors.emplace_back(make_unique<core::Error>(core::Loc::none(), core::ErrorClass{1, core::StrictLevel::True},
                                                 "MyError", vector<core::ErrorSection>(),
                                                 vector<core::AutocorrectSuggestion>(), false));
    {
        core::UnfreezeFileTable fileTableAccess(*gs);
        auto epoch = 0;
        auto file = make_shared<core::File>("foo/bar", "foo", core::File::Type::Normal, epoch);
        auto fref = gs->enterFile(file);

        er.pushDiagnostics(epoch, fref, errors, *gs);
        EXPECT_TRUE(er.getFileErrorStatuses().empty());

        er.commit();
        EXPECT_EQ(fref.id() + 1, er.getFileErrorStatuses().size());
        EXPECT_TRUE(er.getUncommittedFileErrorStatuses().empty());
    }
}
} // namespace sorbet::realmain::lsp::test