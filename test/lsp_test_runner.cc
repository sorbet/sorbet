#include "doctest/doctest.h"
#include <cxxopts.hpp>
// has to go first as it violates our requirements

#include "absl/strings/match.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "common/web_tracer_framework/tracing.h"
#include "main/lsp/LSPConfiguration.h"
#include "test/helpers/expectations.h"
#include "test/helpers/lsp.h"
#include "test/helpers/position_assertions.h"

namespace sorbet::test {
using namespace std;

string singleTest;
string webTraceFile;

bool isTestMessage(const LSPMessage &msg) {
    return msg.isNotification() && msg.method() == LSPMethod::SorbetTypecheckRunInfo;
}

// "Newly pushed diagnostics always replace previously pushed diagnostics. There is no merging that happens
// on the client side." Only keep the newest diagnostics for a file.
void updateDiagnostics(const LSPConfiguration &config, UnorderedMap<string, string> &testFileUris,
                       vector<unique_ptr<LSPMessage>> &responses,
                       map<string, vector<unique_ptr<Diagnostic>>> &diagnostics) {
    for (auto &response : responses) {
        if (isTestMessage(*response)) {
            continue;
        }
        assertNotificationMessage(LSPMethod::TextDocumentPublishDiagnostics, *response);
        auto maybeDiagnosticParams = getPublishDiagnosticParams(response->asNotification());
        REQUIRE(maybeDiagnosticParams.has_value());
        auto &diagnosticParams = *maybeDiagnosticParams;
        string filename;
        if (!absl::StartsWith(diagnosticParams->uri, core::File::URL_PREFIX)) {
            filename = uriToFilePath(config, diagnosticParams->uri);
            INFO(fmt::format("Diagnostic URI is not a test file URI: {}", diagnosticParams->uri));
            CHECK_NE(testFileUris.end(), testFileUris.find(filename));
        } else {
            filename = diagnosticParams->uri;
        }

        // Will explicitly overwrite older diagnostics that are irrelevant.
        vector<unique_ptr<Diagnostic>> copiedDiagnostics;
        for (const auto &d : diagnosticParams->diagnostics) {
            copiedDiagnostics.push_back(d->copy());
        }
        diagnostics[filename] = move(copiedDiagnostics);
    }
}

int countNonTestMessages(const vector<unique_ptr<LSPMessage>> &msgs) {
    int count = 0;
    for (auto &m : msgs) {
        if (!isTestMessage(*m)) {
            count++;
        }
    }
    return count;
}

string documentSymbolsToString(const variant<JSONNullObject, vector<unique_ptr<DocumentSymbol>>> &symbolResult) {
    if (get_if<JSONNullObject>(&symbolResult)) {
        return "null";
    } else {
        auto &symbols = get<vector<unique_ptr<DocumentSymbol>>>(symbolResult);
        return fmt::format("{}",
                           fmt::map_join(symbols, ", ", [](const auto &sym) -> string { return sym->toJSON(true); }));
    }
}

// cf.
// https://github.com/microsoft/vscode/blob/21f7df634a8ac45d1198cb414fe90366f782bcee/src/vs/workbench/api/common/extHostTypes.ts#L1224-L1232
void validateDocumentSymbol(unique_ptr<DocumentSymbol> &sym) {
    REQUIRE(!sym->name.empty());
    REQUIRE(sym->range != nullptr);
    REQUIRE(sym->selectionRange != nullptr);
    REQUIRE(sym->range->start != nullptr);
    REQUIRE(sym->range->end != nullptr);
    REQUIRE(sym->selectionRange->start != nullptr);
    REQUIRE(sym->selectionRange->end != nullptr);

    INFO(fmt::format("Checking range {} contains selectionRange {}", sym->range->toJSON(false),
                     sym->selectionRange->toJSON(false)));
    REQUIRE(sym->range->contains(*sym->selectionRange));

    if (sym->children.has_value()) {
        for (auto &child : *sym->children) {
            validateDocumentSymbol(child);
        }
    }
}

optional<unique_ptr<CodeAction>> resolveCodeAction(LSPWrapper &lspWrapper, int &nextId,
                                                   unique_ptr<CodeAction> codeAction) {
    const auto &config = lspWrapper.config().getClientConfig();
    if (!config.clientCodeActionResolveEditSupport || !config.clientCodeActionDataSupport) {
        return codeAction;
    }

    auto req = make_unique<RequestMessage>("2.0", nextId++, LSPMethod::CodeActionResolve, move(codeAction));
    auto responses = getLSPResponsesFor(lspWrapper, make_unique<LSPMessage>(move(req)));

    responses.erase(
        std::remove_if(responses.begin(), responses.end(), [](const auto &msg) { return !msg->isResponse(); }),
        responses.end());
    {
        INFO("Did not receive exactly one response for a codeAction request.");
        CHECK_EQ(responses.size(), 1);
    }
    if (responses.size() != 1) {
        return nullopt;
    }

    auto &msg = responses.at(0);
    CHECK(msg->isResponse());
    if (!msg->isResponse()) {
        return nullopt;
    }
    auto &response = msg->asResponse();
    auto &receivedResponse = get<optional<unique_ptr<CodeAction>>>(*response.result);
    CHECK(receivedResponse.has_value());
    return move(receivedResponse);
}

optional<vector<unique_ptr<CodeAction>>> requestCodeActions(LSPWrapper &lspWrapper, string fileUri,
                                                            unique_ptr<Range> range, int &nextId,
                                                            vector<CodeActionKind> &selectedCodeActionKinds) {
    vector<unique_ptr<Diagnostic>> diagnostics;

    auto codeActionContext = make_unique<CodeActionContext>(move(diagnostics));
    codeActionContext->only = selectedCodeActionKinds;

    // Unfortunately there's no simpler way to copy the range (yet).
    auto params = make_unique<CodeActionParams>(make_unique<TextDocumentIdentifier>(fileUri), range->copy(),
                                                move(codeActionContext));
    auto req = make_unique<RequestMessage>("2.0", nextId++, LSPMethod::TextDocumentCodeAction, move(params));
    auto responses = getLSPResponsesFor(lspWrapper, make_unique<LSPMessage>(move(req)));
    responses.erase(
        std::remove_if(responses.begin(), responses.end(), [](const auto &msg) { return !msg->isResponse(); }),
        responses.end());
    {
        INFO("Did not receive exactly one response for a codeAction request.");
        CHECK_EQ(responses.size(), 1);
    }
    if (responses.size() != 1) {
        return nullopt;
    }

    auto &msg = responses.at(0);
    CHECK(msg->isResponse());
    if (!msg->isResponse()) {
        return nullopt;
    }

    auto &response = msg->asResponse();
    auto &receivedCodeActionResponse = get<variant<JSONNullObject, vector<unique_ptr<CodeAction>>>>(*response.result);
    CHECK_FALSE(get_if<JSONNullObject>(&receivedCodeActionResponse));
    if (get_if<JSONNullObject>(&receivedCodeActionResponse)) {
        return nullopt;
    }

    auto codeActions = move(get<vector<unique_ptr<CodeAction>>>(receivedCodeActionResponse));
    vector<unique_ptr<CodeAction>> resolvedCodeActions;

    for (auto &ca : codeActions) {
        if (ca->edit.has_value()) {
            resolvedCodeActions.emplace_back(move(ca));
            continue;
        }
        auto resolvedCodeAction = resolveCodeAction(lspWrapper, nextId, move(ca));
        if (resolvedCodeAction.has_value()) {
            resolvedCodeActions.emplace_back(move(resolvedCodeAction.value()));
        }
    }

    for (auto &ca : resolvedCodeActions) {
        CHECK_MESSAGE(ca->edit.has_value(), fmt::format("Code action with kind {} has no edits",
                                                        convertCodeActionKindToString(ca->kind.value())));
    }

    return resolvedCodeActions;
}

void validateCodeActionAbsence(LSPWrapper &lspWrapper, string fileUri, unique_ptr<Range> range, int &nextId,
                               vector<CodeActionKind> &selectedCodeActionKinds,
                               vector<CodeActionKind> &ignoredCodeActionKinds) {
    auto maybeReceivedCodeActions =
        requestCodeActions(lspWrapper, fileUri, range->copy(), nextId, selectedCodeActionKinds);
    if (!maybeReceivedCodeActions.has_value()) {
        return;
    }
    auto receivedCodeActions = move(maybeReceivedCodeActions.value());
    if (!ignoredCodeActionKinds.empty()) {
        vector<CodeActionKind> receivedCodeActionKinds;
        for (const auto &ca : receivedCodeActions) {
            if (!ca->kind.has_value()) {
                continue;
            }
            receivedCodeActionKinds.push_back(ca->kind.value());
        }

        vector<CodeActionKind> ignoredCodeActionsDiff;
        set_intersection(receivedCodeActionKinds.begin(), receivedCodeActionKinds.end(), ignoredCodeActionKinds.begin(),
                         ignoredCodeActionKinds.end(), back_inserter(ignoredCodeActionsDiff));

        REQUIRE_MESSAGE(ignoredCodeActionsDiff.empty(),
                        fmt::format("Received unexpected code actions with kinds: {}",
                                    fmt::map_join(ignoredCodeActionsDiff, ", ", [](auto codeAction) {
                                        return convertCodeActionKindToString(codeAction);
                                    })));
    }
}

void validateCodeActions(LSPWrapper &lspWrapper, Expectations &test, string fileUri, unique_ptr<Range> range,
                         int &nextId, vector<CodeActionKind> &selectedCodeActionKinds,
                         vector<CodeActionKind> &ignoredCodeActionKinds,
                         vector<shared_ptr<ApplyCodeActionAssertion>> &applyCodeActionAssertions,
                         string codeActionDescription, bool assertAllChanges) {
    auto isSelectedKind = [&selectedCodeActionKinds](CodeActionKind kind) {
        return count(selectedCodeActionKinds.begin(), selectedCodeActionKinds.end(), kind) != 0;
    };

    UnorderedMap<string, unique_ptr<CodeAction>> receivedCodeActionsByTitle;

    auto receivedCodeActions =
        requestCodeActions(lspWrapper, fileUri, range->copy(), nextId, selectedCodeActionKinds).value();
    unique_ptr<CodeAction> sourceLevelCodeAction;

    // One code action should be a 'source' level code action. Remove it.
    if (!receivedCodeActions.empty() && isSelectedKind(CodeActionKind::Quickfix)) {
        for (auto it = receivedCodeActions.begin(); it != receivedCodeActions.end();) {
            if ((*it)->kind == CodeActionKind::SourceFixAllSorbet) {
                INFO("Received multiple source-level code actions");
                CHECK_EQ(sourceLevelCodeAction, nullptr);
                sourceLevelCodeAction = move(*it);
                // Remove from vector and continue
                it = receivedCodeActions.erase(it);
            } else {
                it++;
            }
        }
        INFO("Expected one source-level code action for code action request");
        CHECK_NE(sourceLevelCodeAction, nullptr);
    }

    for (auto &codeAction : receivedCodeActions) {
        if (!isSelectedKind(codeAction->kind.value())) {
            continue;
        }
        // We send two identical "Apply all Sorbet autocorrects" code actions with different kinds: One is a
        // Source, the other is a Quickfix. This logic strips out the quickfix.
        if (sourceLevelCodeAction != nullptr && codeAction->title == sourceLevelCodeAction->title) {
            continue;
        }

        bool codeActionTitleUnique =
            receivedCodeActionsByTitle.find(codeAction->title) == receivedCodeActionsByTitle.end();
        CHECK_MESSAGE(codeActionTitleUnique, "Found code action with duplicate title: " << codeAction->title);
        if (codeActionTitleUnique) {
            receivedCodeActionsByTitle[codeAction->title] = move(codeAction);
        }
    }

    uint32_t receivedCodeActionsCount = receivedCodeActionsByTitle.size();
    vector<shared_ptr<ApplyCodeActionAssertion>> matchedCodeActionAssertions;

    // Test code action assertions matching the range of this error.
    auto it = applyCodeActionAssertions.begin();
    while (it != applyCodeActionAssertions.end()) {
        auto codeActionAssertion = it->get();
        if (!(range->start->cmp(*codeActionAssertion->range->start) <= 0 &&
              range->end->cmp(*codeActionAssertion->range->end) >= 0)) {
            ++it;
            continue;
        }

        // Ensure we received a code action matching the assertion.
        auto it2 = receivedCodeActionsByTitle.find(codeActionAssertion->title);
        {
            INFO(fmt::format(
                "Did not receive code action matching assertion `{}` for error or selected code action `{}`...",
                codeActionAssertion->toString(), codeActionDescription));
            INFO("(If this was the expected behavior, add `# assert-no-code-action: $CODE_ACTION_KIND` to your "
                 "testcase)");
            REQUIRE_NE(it2, receivedCodeActionsByTitle.end());
        }

        // Ensure that the received code action applies correctly.
        if (it2 != receivedCodeActionsByTitle.end()) {
            auto codeAction = move(it2->second);
            if (assertAllChanges) {
                codeActionAssertion->checkAll(test.sourceFileContents, lspWrapper, *codeAction.get());
            } else {
                codeActionAssertion->check(test.sourceFileContents, lspWrapper, *codeAction.get());
            }

            // Some bookkeeping to make surfacing errors re. extra/insufficient
            // apply-code-action annotations easier.
            receivedCodeActionsByTitle.erase(it2);

            if (isSelectedKind(codeAction->kind.value())) {
                (*it)->kind = codeAction->kind;
                matchedCodeActionAssertions.emplace_back(*it);
                it = applyCodeActionAssertions.erase(it);
            }
        } else {
            ++it;
        }
    }

    if (matchedCodeActionAssertions.size() > receivedCodeActionsCount) {
        FAIL_CHECK(fmt::format("Found apply-code-action assertions without "
                               "corresponding code actions from the server:\n{}",
                               fmt::map_join(applyCodeActionAssertions, ", ",
                                             [](const auto &assertion) -> string { return assertion->toString(); })));
    } else if (matchedCodeActionAssertions.size() < receivedCodeActionsCount) {
        FAIL_CHECK(fmt::format("Received code actions without corresponding apply-code-action assertions:\n{}",
                               fmt::map_join(receivedCodeActionsByTitle, "\n",
                                             [](const auto &action) -> string { return action.second->toJSON(); })));
    }
}

void testQuickFixCodeActions(LSPWrapper &lspWrapper, Expectations &test, const vector<string> &filenames,
                             vector<shared_ptr<RangeAssertion>> &assertions, UnorderedMap<string, string> &testFileUris,
                             int &nextId) {
    UnorderedMap<string, vector<shared_ptr<ApplyCodeActionAssertion>>> applyCodeActionAssertionsByFilename;
    for (auto &assertion : assertions) {
        if (auto applyCodeActionAssertion = dynamic_pointer_cast<ApplyCodeActionAssertion>(assertion)) {
            applyCodeActionAssertionsByFilename[applyCodeActionAssertion->filename].push_back(applyCodeActionAssertion);
        }
    }

    auto selectedCodeActions =
        StringPropertyAssertions::getValues("selective-apply-code-action", assertions).value_or(vector<string>{});
    if (applyCodeActionAssertionsByFilename.empty() && selectedCodeActions.empty()) {
        return;
    }

    {
        INFO("No code actions provided for the selective-apply-code-action assertion. Correct usage example "
             "`selective-apply-code-action: quickfix, quickfix.refactor`");
        CHECK(!selectedCodeActions.empty());
    }

    vector<CodeActionKind> selectedCodeActionKinds;
    transform(selectedCodeActions.begin(), selectedCodeActions.end(), back_inserter(selectedCodeActionKinds),
              getCodeActionKind);

    auto ignoredCodeActionAssertions =
        StringPropertyAssertions::getValues("assert-no-code-action", assertions).value_or(vector<string>{});
    vector<CodeActionKind> ignoredCodeActionKinds;
    transform(ignoredCodeActionAssertions.begin(), ignoredCodeActionAssertions.end(),
              back_inserter(ignoredCodeActionKinds), getCodeActionKind);

    auto errors = RangeAssertion::getErrorAssertions(assertions);
    UnorderedMap<string, std::vector<std::shared_ptr<RangeAssertion>>> errorsByFilename;
    for (auto &error : errors) {
        errorsByFilename[error->filename].emplace_back(error);
    }

    for (auto &filename : filenames) {
        auto applyCodeActionAssertions = applyCodeActionAssertionsByFilename[filename];
        auto fileUri = testFileUris[filename];

        if (ignoredCodeActionKinds.empty()) {
            // Request code actions for each of this file's error.
            for (auto &error : errorsByFilename[filename]) {
                validateCodeActions(lspWrapper, test, fileUri, error->range->copy(), nextId, selectedCodeActionKinds,
                                    ignoredCodeActionKinds, applyCodeActionAssertions, error->toString(), false);
            }

            // This weird loop is here because `validateCodeActions` erases the elements from
            // `applyCodeActionAssertions` and we are iterating over that container
            while (!applyCodeActionAssertions.empty()) {
                auto initialSize = applyCodeActionAssertions.size();
                if (applyCodeActionAssertions.empty()) {
                    break;
                }
                auto codeActionAssertion = applyCodeActionAssertions.at(0);
                validateCodeActions(lspWrapper, test, fileUri, codeActionAssertion->range->copy(), nextId,
                                    selectedCodeActionKinds, ignoredCodeActionKinds, applyCodeActionAssertions,
                                    codeActionAssertion->toString(), true);
                REQUIRE_LT(applyCodeActionAssertions.size(), initialSize);
            }

            // We've already removed any code action assertions that matches a received code action assertion.
            // Any remaining are therefore extraneous.
            INFO(fmt::format("Found extraneous apply-code-action assertions:\n{}",
                             fmt::map_join(applyCodeActionAssertions, "\n",
                                           [](const auto &assertion) -> string { return assertion->toString(); })));
            CHECK_EQ(applyCodeActionAssertions.size(), 0);
        } else {
            for (auto assertion : applyCodeActionAssertions) {
                validateCodeActionAbsence(lspWrapper, fileUri, assertion->range->copy(), nextId,
                                          selectedCodeActionKinds, ignoredCodeActionKinds);
            }
        }
    }
}

void testDocumentSymbols(LSPWrapper &lspWrapper, Expectations &test, int &nextId, string_view uri,
                         string_view testFile) {
    auto expectationFileName = test.expectations["document-symbols"][testFile];
    if (expectationFileName.empty()) {
        // No .exp file found; nothing to do.
        return;
    }

    auto params = make_unique<DocumentSymbolParams>(make_unique<TextDocumentIdentifier>(string(uri)));
    auto req = make_unique<RequestMessage>("2.0", nextId++, LSPMethod::TextDocumentDocumentSymbol, move(params));
    auto responses = getLSPResponsesFor(lspWrapper, make_unique<LSPMessage>(move(req)));
    responses.erase(
        std::remove_if(responses.begin(), responses.end(), [](const auto &msg) { return !msg->isResponse(); }),
        responses.end());
    {
        INFO("Did not receive exactly one response for a documentSymbols request.");
        REQUIRE_EQ(responses.size(), 1);
    }
    auto &msg = responses.at(0);
    REQUIRE(msg->isResponse());
    auto &response = msg->asResponse();
    REQUIRE_MESSAGE(response.result, "Document symbols request returned error: " << msg->toJSON());
    auto &receivedSymbolResponse = get<variant<JSONNullObject, vector<unique_ptr<DocumentSymbol>>>>(*response.result);

    auto expectedSymbolsPath = test.folder + expectationFileName;
    auto expected = LSPMessage::fromClient(FileOps::read(expectedSymbolsPath));
    auto &expectedResp = expected->asResponse();
    auto &expectedSymbolResponse =
        get<variant<JSONNullObject, vector<unique_ptr<DocumentSymbol>>>>(*expectedResp.result);

    // Simple string comparison, just like other *.exp files.
    CHECK_EQ_DIFF(documentSymbolsToString(expectedSymbolResponse), documentSymbolsToString(receivedSymbolResponse),
                  "Mismatch on: " + expectedSymbolsPath);

    // VSCode validates document symbols, so we should too.
    if (auto *syms = get_if<vector<unique_ptr<DocumentSymbol>>>(&receivedSymbolResponse)) {
        for (auto &sym : *syms) {
            validateDocumentSymbol(sym);
        }
    }
}

void testDocumentFormatting(LSPWrapper &lspWrapper, Expectations &test, int &nextId, string_view uri,
                            string_view testFile) {
    auto expectationFileName = test.expectations["document-formatting-rubyfmt"][testFile];
    if (expectationFileName.empty()) {
        // No .exp file found; nothing to do.
        return;
    }

    auto params = make_unique<DocumentFormattingParams>(make_unique<TextDocumentIdentifier>(string(uri)),
                                                        make_unique<FormattingOptions>(4, 4));
    auto req = make_unique<RequestMessage>("2.0", nextId++, LSPMethod::TextDocumentFormatting, move(params));
    auto responses = getLSPResponsesFor(lspWrapper, make_unique<LSPMessage>(move(req)));

    // successful response
    if (responses.at(0)->isResponse()) {
        INFO("Did not receive exactly one response for a documentFormatting request.");
        REQUIRE_EQ(responses.size(), 1);
        auto &msg = responses.at(0);
        REQUIRE(msg->isResponse());
        auto &response = msg->asResponse();
        REQUIRE_MESSAGE(response.result, "Document formatting request returned error: " << msg->toJSON());
        auto &receivedFormattingResponse = get<variant<JSONNullObject, vector<unique_ptr<TextEdit>>>>(*response.result);
        auto expectedOutput = FileOps::read(test.folder + expectationFileName);
        if (auto *edits = get_if<vector<unique_ptr<TextEdit>>>(&receivedFormattingResponse)) {
            // We can support multiple edits, but right now the impl only returns one.
            REQUIRE_EQ(1, edits->size());
            auto formattedText = (*edits)[0]->newText;
            REQUIRE_EQ(expectedOutput, formattedText);
        } else {
            // Syntax error responses return null
            auto isJSONNullObject = std::holds_alternative<JSONNullObject>(receivedFormattingResponse);
            REQUIRE(isJSONNullObject);
            REQUIRE_EQ(expectedOutput, "");
        }
    } else {
        // Error responses return both a user notification and an LSP error
        REQUIRE_EQ(responses.size(), 2);
        REQUIRE(responses.at(0)->isNotification());
        auto &errorMsg = responses.at(1);
        REQUIRE(errorMsg->isResponse());
        auto &receivedErrorResponse = *errorMsg->asResponse().error;
        auto expectedOutput = FileOps::read(test.folder + expectationFileName);
        REQUIRE_EQ(expectedOutput, receivedErrorResponse->message);
    }
}

enum class ExpectDiagnosticMessages {
    No = 0,
    Yes = 1,
};

// Check responses for a SorbetTypecheckRunInfo notification with the expected
// status.  If such a notification is found, run custom logic via the handler.
void verifyTypecheckRunInfo(const string &errorPrefix, vector<unique_ptr<LSPMessage>> &responses,
                            SorbetTypecheckRunStatus expectedStatus, ExpectDiagnosticMessages expectDiagnostics,
                            function<void(unique_ptr<SorbetTypecheckRunInfo> &)> handler) {
    bool foundTypecheckRunInfo = false;

    for (auto &r : responses) {
        if (!r->isNotification()) {
            FAIL_CHECK(errorPrefix << "Sorbet sent an unexpected message");
            continue;
        }

        if (r->method() != LSPMethod::SorbetTypecheckRunInfo) {
            if (expectDiagnostics == ExpectDiagnosticMessages::Yes &&
                r->method() == LSPMethod::TextDocumentPublishDiagnostics) {
                continue;
            }
            FAIL_CHECK(errorPrefix << fmt::format("Unexpected message response to file update of type {}:\n{}",
                                                  convertLSPMethodToString(r->method()), r->toJSON()));
            continue;
        }

        auto &params = get<unique_ptr<SorbetTypecheckRunInfo>>(r->asNotification().params);
        if (params->status == expectedStatus) {
            foundTypecheckRunInfo = true;
            handler(params);
        }
    }

    if (!foundTypecheckRunInfo) {
        FAIL_CHECK(errorPrefix << "Sorbet did not send expected typechecking metadata.");
    }
}

TEST_CASE("LSPTest") {
    /** The path to the test Ruby files on disk */
    vector<std::string> filenames;
    std::unique_ptr<LSPWrapper> lspWrapper;

    /** Test expectations. */
    Expectations test = Expectations::getExpectations(singleTest);

    /** All test assertions ordered by (filename, range, message). */
    std::vector<std::shared_ptr<RangeAssertion>> assertions = RangeAssertion::parseAssertions(test.sourceFileContents);

    /** The next ID to use when sending an LSP message. */
    int nextId = 0;

    for (auto &sourceFile : test.sourceFiles) {
        filenames.push_back(test.folder + sourceFile);
    }

    // Initialize lspWrapper.
    {
        shared_ptr<realmain::options::Options> opts = make_shared<realmain::options::Options>();
        opts->noStdlib = BooleanPropertyAssertion::getValue("no-stdlib", assertions).value_or(false);
        opts->ruby3KeywordArgs =
            BooleanPropertyAssertion::getValue("experimental-ruby3-keyword-args", assertions).value_or(false);
        opts->typedSuper = BooleanPropertyAssertion::getValue("typed-super", assertions).value_or(true);
        // TODO(jez) Allow suppressPayloadSuperclassRedefinitionFor in a testdata test assertion?
        opts->stripeMode = BooleanPropertyAssertion::getValue("stripe-mode", assertions).value_or(false);
        opts->outOfOrderReferenceChecksEnabled =
            BooleanPropertyAssertion::getValue("check-out-of-order-constant-references", assertions).value_or(false);
        opts->requiresAncestorEnabled =
            BooleanPropertyAssertion::getValue("enable-experimental-requires-ancestor", assertions).value_or(false);
        opts->lspExtractToVariableEnabled =
            BooleanPropertyAssertion::getValue("enable-experimental-lsp-extract-to-variable", assertions)
                .value_or(false);
        opts->stripePackages = BooleanPropertyAssertion::getValue("enable-packager", assertions).value_or(false);

        if (opts->stripePackages) {
            auto extraDirUnderscore =
                StringPropertyAssertion::getValue("extra-package-files-directory-prefix-underscore", assertions);
            if (extraDirUnderscore.has_value()) {
                opts->extraPackageFilesDirectoryUnderscorePrefixes.emplace_back(extraDirUnderscore.value());
            }
            auto extraDirSlash =
                StringPropertyAssertion::getValue("extra-package-files-directory-prefix-slash", assertions);
            if (extraDirSlash.has_value()) {
                opts->extraPackageFilesDirectorySlashPrefixes.emplace_back(extraDirSlash.value());
            }
            auto skipImportVisibility =
                StringPropertyAssertion::getValue("allow-relaxed-packager-checks-for", assertions);
            if (skipImportVisibility.has_value()) {
                opts->allowRelaxedPackagerChecksFor.emplace_back(skipImportVisibility.value());
            }
        }
        opts->disableWatchman = true;
        opts->rubyfmtPath = "test/testdata/lsp/rubyfmt-stub/rubyfmt";

        // Set to a number that is reasonable large for tests, but small enough that we can have a test to handle
        // this edge case. If you change this number, `fast_path/{too_many_files,not_enough_files,initialize}` will
        // need to be changed as well.
        opts->lspMaxFilesOnFastPath = 10;
        lspWrapper = SingleThreadedLSPWrapper::create("", move(opts));
        lspWrapper->enableAllExperimentalFeatures();
    }

    if (test.expectations.contains("autogen")) {
        // Some autogen tests assume that some errors will occur from the resolver step, others assume the resolver
        // won't run.
        if (!RangeAssertion::getErrorAssertions(assertions).empty()) {
            // ...and stop after the resolver phase if there are errors
            lspWrapper->opts->stopAfterPhase = realmain::options::Phase::RESOLVER;
        } else {
            // ...and stop after the namer phase if there are no errors
            lspWrapper->opts->stopAfterPhase = realmain::options::Phase::NAMER;
        }
    }

    const auto &config = lspWrapper->config();
    auto shouldUseCodeActionResolve =
        BooleanPropertyAssertion::getValue("use-code-action-resolve", assertions).value_or(true);

    // Perform initialize / initialized handshake.
    {
        string rootPath = fmt::format("/Users/{}/stripe/sorbet", std::getenv("USER"));
        string rootUri = fmt::format("file://{}", rootPath);
        auto sorbetInitOptions = make_unique<SorbetInitializationOptions>();
        sorbetInitOptions->enableTypecheckInfo = true;
        sorbetInitOptions->highlightUntyped =
            BooleanPropertyAssertion::getValue("highlight-untyped-values", assertions).value_or(false);
        sorbetInitOptions->enableTypedFalseCompletionNudges =
            BooleanPropertyAssertion::getValue("enable-typed-false-completion-nudges", assertions).value_or(true);
        auto initializedResponses = initializeLSP(rootPath, rootUri, *lspWrapper, nextId, true,
                                                  shouldUseCodeActionResolve, move(sorbetInitOptions));
        INFO("Should not receive any response to 'initialized' message.");
        CHECK_EQ(0, countNonTestMessages(initializedResponses));
    }

    // filename => URI; do post-initialization so LSPConfiguration has rootUri set.
    UnorderedMap<string, string> testFileUris;
    for (auto &filename : filenames) {
        testFileUris[filename] = filePathToUri(config, filename);
    }

    // Tell LSP that we opened a bunch of brand new, empty files (the test files).
    {
        for (auto &filename : filenames) {
            auto params = make_unique<DidOpenTextDocumentParams>(
                make_unique<TextDocumentItem>(testFileUris[filename], "ruby", 1, ""));
            auto responses = getLSPResponsesFor(*lspWrapper, make_unique<LSPMessage>(make_unique<NotificationMessage>(
                                                                 "2.0", LSPMethod::TextDocumentDidOpen, move(params))));
            // Sorbet will complain about missing packages in packaging mode. Ignore them.
            if (!lspWrapper->opts->stripePackages) {
                INFO("Should not receive any response to opening an empty file.");
                CHECK_EQ(0, countNonTestMessages(responses));
            }
        }
    }

    // filename => diagnostics for file (persist for fast path tests)
    map<string, vector<unique_ptr<Diagnostic>>> diagnostics;

    // Tell LSP that the new files now have the contents from the test files on disk.
    {
        bool slowPathPassed = true;
        bool skipFastPath = BooleanPropertyAssertion::getValue("disable-fast-path", assertions).value_or(false);
        vector<string> errorPrefixes = {"", "[After running fast path] "};
        // Run changes through LSP twice: The first time is a slow path, the second time is a fast path.
        // Surfaces errors that occur due to differences in how slow and fast paths run.
        // Skip the second iteration if slow path fails to avoid printing out duplicate errors.
        for (int i = 0; i < (skipFastPath ? 1 : 2) && slowPathPassed; i++) {
            // Print a helpful message when a test fails during the fast path loop.
            INFO((i == 1 ? "Note: To disable fast path tests, add `# disable-fast-path: true` to the file." : ""));
            vector<unique_ptr<LSPMessage>> updates;
            for (auto &filename : filenames) {
                auto textDocContents = test.sourceFileContents[filename]->source();
                updates.push_back(makeChange(testFileUris[filename], textDocContents, 2 + i));
            }
            auto responses = getLSPResponsesFor(*lspWrapper, move(updates));
            updateDiagnostics(config, testFileUris, responses, diagnostics);
            bool errorAssertionsPassed = ErrorAssertion::checkAll(
                test.sourceFileContents, RangeAssertion::getErrorAssertions(assertions), diagnostics, errorPrefixes[i]);

            bool untypedAssertionsPassed =
                UntypedAssertion::checkAll(test.sourceFileContents, RangeAssertion::getUntypedAssertions(assertions),
                                           diagnostics, errorPrefixes[i]);

            slowPathPassed = errorAssertionsPassed && untypedAssertionsPassed;
        }
    }

    for (auto &filename : filenames) {
        testDocumentSymbols(*lspWrapper, test, nextId, testFileUris[filename], filename);
        testDocumentFormatting(*lspWrapper, test, nextId, testFileUris[filename], filename);
    }
    testQuickFixCodeActions(*lspWrapper, test, filenames, assertions, testFileUris, nextId);

    // Usage and def assertions
    {
        // Sort by symbol.
        // symbol => [ (version => vector<DefAssertion>), (DefAssertion | UsageAssertion)[] ]
        // Note: Using a vector in pair since order matters; assertions are ordered by location, which
        // is used when comparing against LSP responses.
        UnorderedMap<string,
                     pair<UnorderedMap<int, vector<shared_ptr<DefAssertion>>>, vector<shared_ptr<RangeAssertion>>>>
            defUsageMap;

        // symbol => [ TypeDefAssertion[], TypeAssertion[] ]
        //
        // (The first element of the pair is only ever TypeDefAssertions but we only ever care that they're
        // RangeAssertions, so rather than fiddle with up casting, we'll just make the whole vector RangeAssertions)
        UnorderedMap<string, pair<vector<shared_ptr<RangeAssertion>>, vector<shared_ptr<TypeAssertion>>>> typeDefMap;

        // symbol => [ ImplementationAssertion[], FindImplementationAssertion[] ]
        UnorderedMap<string,
                     pair<vector<shared_ptr<ImplementationAssertion>>, vector<shared_ptr<FindImplementationAssertion>>>>
            implementationMap;
        for (auto &assertion : assertions) {
            if (auto defAssertion = dynamic_pointer_cast<DefAssertion>(assertion)) {
                auto &entry = defUsageMap[defAssertion->symbol];
                auto &defMap = entry.first;
                defMap[defAssertion->version].emplace_back(defAssertion);

                // If this is a definition that corresponds to an argument default, don't add it to the list of
                // assertions to be explicitly checked -- it's only present for validating usage queries.
                if (!defAssertion->isDefaultArgValue) {
                    entry.second.emplace_back(defAssertion);
                }
            } else if (auto usageAssertion = dynamic_pointer_cast<UsageAssertion>(assertion)) {
                auto &entry = defUsageMap[usageAssertion->symbol];
                entry.second.emplace_back(usageAssertion);
            } else if (auto typeDefAssertion = dynamic_pointer_cast<TypeDefAssertion>(assertion)) {
                auto &[typeDefs, _typeAssertions] = typeDefMap[typeDefAssertion->symbol];
                typeDefs.emplace_back(typeDefAssertion);
            } else if (auto typeAssertion = dynamic_pointer_cast<TypeAssertion>(assertion)) {
                auto &[_typeDefs, typeAssertions] = typeDefMap[typeAssertion->symbol];
                typeAssertions.emplace_back(typeAssertion);
            } else if (auto implAssertion = dynamic_pointer_cast<ImplementationAssertion>(assertion)) {
                auto &[impls, _implAssertions] = implementationMap[implAssertion->symbol];
                impls.emplace_back(implAssertion);
            } else if (auto findImplAssertion = dynamic_pointer_cast<FindImplementationAssertion>(assertion)) {
                auto &[_impls, implAssertions] = implementationMap[findImplAssertion->symbol];
                implAssertions.emplace_back(findImplAssertion);
            }
        }

        // Check each def/usage assertion.
        for (auto &entry : defUsageMap) {
            auto &entryAssertions = entry.second.second;
            // Sort def|usage assertions in (filename, range) order
            fast_sort(entryAssertions, RangeAssertion::compareByRange);
            // Sort def assertions in (filename, range) order
            for (auto &versionDefEntry : entry.second.first) {
                fast_sort(versionDefEntry.second, RangeAssertion::compareByRange);
            }

            auto &defAssertions = entry.second.first;
            // Shouldn't be possible to have an entry with 0 assertions, but explicitly check anyway.
            CHECK_GE(entryAssertions.size(), 1);

            // Collect importUsageAssertions into a separate collection to handle them differently.
            std::vector<shared_ptr<RangeAssertion>> importUsageAssertions;
            entryAssertions.erase(std::remove_if(entryAssertions.begin(), entryAssertions.end(),
                                                 [&](auto &assertion) -> bool {
                                                     if (dynamic_pointer_cast<ImportUsageAssertion>(assertion)) {
                                                         importUsageAssertions.emplace_back(assertion);
                                                         return true;
                                                     }

                                                     return false;
                                                 }),
                                  entryAssertions.end());

            for (auto &assertion : entryAssertions) {
                string_view symbol;
                vector<int> versions;
                if (auto defAssertion = dynamic_pointer_cast<DefAssertion>(assertion)) {
                    // Some definition locations are not the definition of themselves.
                    if (!defAssertion->isDefOfSelf || defAssertion->isDefaultArgValue) {
                        continue;
                    }
                    versions.push_back(defAssertion->version);
                    symbol = defAssertion->symbol;
                } else if (auto usageAssertion = dynamic_pointer_cast<UsageAssertion>(assertion)) {
                    versions = usageAssertion->versions;
                    symbol = usageAssertion->symbol;
                }

                vector<shared_ptr<DefAssertion>> defs;
                for (auto version : versions) {
                    auto entry = defAssertions.find(version);
                    if (entry != defAssertions.end()) {
                        defs.insert(defs.end(), entry->second.begin(), entry->second.end());
                    } else {
                        ADD_FAIL_CHECK_AT(
                            assertion->filename.c_str(), assertion->range->start->line + 1,
                            fmt::format("Found usage comment for label {0} version {1} without matching def "
                                        "comment. Please add a `# "
                                        "^^ def: {0} {1}` assertion that points to the definition of the "
                                        "pointed-to thing being used.",
                                        symbol, version));
                    }
                }

                // if there were versions that weren't present in the defAssertions map, an error will have been raised,
                // but the test will proceed to this point.
                if (defs.empty()) {
                    ADD_FAIL_AT(assertion->filename.c_str(), assertion->range->start->line + 1,
                                fmt::format("Found no def comments for usage comment `{}`", symbol));
                }

                auto queryLoc = assertion->getLocation(config);

                // Check that a definition request at this location returns defs.
                DefAssertion::check(test.sourceFileContents, *lspWrapper, nextId, *queryLoc, defs);
                if (dynamic_pointer_cast<ImportAssertion>(assertion)) {
                    // For an ImportAssertion, check that a reference request at this location returns
                    // importUsageAssertions.
                    UsageAssertion::check(test.sourceFileContents, *lspWrapper, nextId, symbol, *queryLoc,
                                          importUsageAssertions);
                } else if (dynamic_pointer_cast<GoToDefSpecialAssertion>(assertion) == nullptr) {
                    // For a regular UsageAssertion, check that a reference request at this location returns
                    // entryAssertions.
                    UsageAssertion::check(test.sourceFileContents, *lspWrapper, nextId, symbol, *queryLoc,
                                          entryAssertions);
                    // Check that a highlight request at this location returns all of the entryAssertions for the same
                    // file as the request.
                    vector<shared_ptr<RangeAssertion>> filteredEntryAssertions;
                    for (auto &e : entryAssertions) {
                        if (absl::StartsWith(e->getLocation(config)->uri, queryLoc->uri)) {
                            filteredEntryAssertions.push_back(e);
                        }
                    }
                    UsageAssertion::checkHighlights(test.sourceFileContents, *lspWrapper, nextId, symbol, *queryLoc,
                                                    filteredEntryAssertions);
                }
            }
        }

        // Check each type-def/type assertion.
        for (auto &[symbol, typeDefAndAssertions] : typeDefMap) {
            auto &[typeDefs, typeAssertions] = typeDefAndAssertions;
            for (auto &typeAssertion : typeAssertions) {
                auto queryLoc = typeAssertion->getLocation(config);
                // Check that a type definition request at this location returns type-def.
                TypeDefAssertion::check(test.sourceFileContents, *lspWrapper, nextId, symbol, *queryLoc, typeDefs);
            }
        }

        // Check each find/implementation assertion.
        for (auto &[symbol, implsAndAssertions] : implementationMap) {
            auto &[impls, implAssertions] = implsAndAssertions;
            for (auto &implAssertion : implAssertions) {
                auto queryLoc = implAssertion->getLocation(config);
                FindImplementationAssertion::check(test.sourceFileContents, *lspWrapper, nextId, symbol, *queryLoc,
                                                   impls);
            }
        }
    }

    // Hover assertions
    HoverAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

    // Hover multiline assertions
    HoverLineAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

    // sorbet/showSymbol assertions
    ShowSymbolAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

    // Completion assertions
    CompletionAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);
    ApplyCompletionAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

    // Rename assertion
    ApplyRenameAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

    // Workspace Symbol assertions
    SymbolSearchAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

    // Fast path tests: Asserts that certain changes take the fast/slow path, and produce any expected diagnostics.
    {
        // sourceLSPFileUpdates is unordered (and we can't use an ordered map unless we make its contents `const`)
        // Sort by version.
        vector<int> sortedUpdates;
        const int baseVersion = 4;
        for (auto &update : test.sourceLSPFileUpdates) {
            sortedUpdates.push_back(update.first);
        }
        fast_sort(sortedUpdates);

        // Apply updates in order.
        for (auto version : sortedUpdates) {
            auto errorPrefix = fmt::format("[*.{}.rbupdate] ", version);
            const auto &updates = test.sourceLSPFileUpdates[version];
            vector<unique_ptr<LSPMessage>> lspUpdates;
            UnorderedMap<std::string, std::shared_ptr<core::File>> updatesAndContents;

            for (const auto &update : updates) {
                auto originalFile = test.folder + update.first;
                auto updateFile = test.folder + update.second;
                auto fileContents = FileOps::read(updateFile);
                updatesAndContents[originalFile] =
                    make_shared<core::File>(string(originalFile), move(fileContents), core::File::Type::Normal);
            }
            auto assertions = RangeAssertion::parseAssertions(updatesAndContents);
            for (const auto &update : updates) {
                auto originalFile = test.folder + update.first;
                auto updateFile = test.folder + update.second;

                auto fileContents = updatesAndContents[originalFile]->source();
                if (!absl::StrContains(fileContents, "# exclude-from-file-update: true")) {
                    // Allow some files in the version to only be used for the sake of asserting
                    // errors that occur after the update, not extending the file update itself.
                    lspUpdates.push_back(makeChange(testFileUris[originalFile], fileContents, baseVersion + version));
                }
            }
            auto assertFastPath = FastPathAssertion::get(assertions);
            auto assertSlowPath = BooleanPropertyAssertion::getValue("assert-slow-path", assertions);
            auto responses = getLSPResponsesFor(*lspWrapper, move(lspUpdates));

            // Ignore started messages.  Note that cancelation messages cannot occur
            // in this codepath since we are running in single-threaded mode.
            verifyTypecheckRunInfo(
                errorPrefix, responses, SorbetTypecheckRunStatus::Ended, ExpectDiagnosticMessages::Yes,
                [&errorPrefix, assertSlowPath, &assertFastPath, &test,
                 &version](unique_ptr<SorbetTypecheckRunInfo> &params) -> void {
                    auto validateAssertions = [&params, &errorPrefix](TypecheckingPath path, string actualPath,
                                                                      bool assertValue, string expectedPath) {
                        auto isSelectedPath = params->typecheckingPath == path;
                        if (isSelectedPath && assertValue) {
                            INFO(errorPrefix << fmt::format("Expected Sorbet to take {} path, but it took the {} path.",
                                                            expectedPath, actualPath));
                            CHECK_NE(isSelectedPath, assertValue);
                        } else if (!isSelectedPath && !assertValue) {
                            INFO(errorPrefix << fmt::format("Expected Sorbet to take {} path, but it took the {} path.",
                                                            expectedPath, actualPath));
                            CHECK_NE(isSelectedPath, assertValue);
                        }
                    };
                    if (assertSlowPath.has_value()) {
                        validateAssertions(TypecheckingPath::Fast, "fast", assertSlowPath.value(), "slow");
                    }
                    if (assertFastPath.has_value()) {
                        (*assertFastPath)->check(*params, test.folder, version, errorPrefix);
                    }
                });

            updateDiagnostics(config, testFileUris, responses, diagnostics);

            for (auto &update : updates) {
                auto originalFile = test.folder + update.first;
                auto updateFile = test.folder + update.second;
                testDocumentSymbols(*lspWrapper, test, nextId, testFileUris[originalFile], updateFile);
            }

            const bool passed = ErrorAssertion::checkAll(
                updatesAndContents, RangeAssertion::getErrorAssertions(assertions), diagnostics, errorPrefix);

            if (!passed) {
                // Abort if an update fails its assertions, as subsequent updates will likely fail as well.
                break;
            }

            // Check any new HoverAssertions in the updates.
            HoverAssertion::checkAll(assertions, updatesAndContents, *lspWrapper, nextId);

            // Check and new HoverMultilneAsserions assertions
            HoverLineAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);
        }
    }

    if (!::sorbet::test::webTraceFile.empty()) {
        auto counters = getAndClearThreadCounters();
        web_tracer_framework::Tracing::storeTraces(counters, ::sorbet::test::webTraceFile);
    }
}
} // namespace sorbet::test

int main(int argc, char *argv[]) {
    cxxopts::Options options("lsp_test_corpus", "Test corpus for Sorbet's language server");
    options.allow_unrecognised_options().add_options()("single_test", "run over single test.",
                                                       cxxopts::value<std::string>()->default_value(""), "testpath");
    options.add_options("advanced")("web-trace-file", "Web trace file. For use with chrome about://tracing",
                                    cxxopts::value<std::string>()->default_value(""), "file");
    auto res = options.parse(argc, argv);

    if (res.count("single_test") != 1) {
        printf("--single_test=<filename> argument expected\n");
        return 1;
    }

    sorbet::test::singleTest = res["single_test"].as<std::string>();
    sorbet::test::webTraceFile = res["web-trace-file"].as<std::string>();

    doctest::Context context(argc, argv);
    return context.run();
}
