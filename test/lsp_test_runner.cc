#include "doctest.h"
#include <cxxopts.hpp>
// has to go first as it violates our requirements

#include "absl/strings/match.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/treemap/treemap.h"
#include "cfg/CFG.h"
#include "cfg/builder/builder.h"
#include "class_flatten/class_flatten.h"
#include "common/FileOps.h"
#include "common/common.h"
#include "common/formatting.h"
#include "common/sort.h"
#include "common/web_tracer_framework/tracing.h"
#include "core/Error.h"
#include "test/helpers/expectations.h"
#include "test/helpers/lsp.h"
#include "test/helpers/position_assertions.h"
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

namespace sorbet::test {
namespace spd = spdlog;
using namespace std;

string singleTest;
string webTraceFile;

UnorderedSet<string> knownExpectations = {
    "parse-tree",       "parse-tree-json", "parse-tree-whitequark", "desugar-tree", "desugar-tree-raw", "rewrite-tree",
    "rewrite-tree-raw", "index-tree",      "index-tree-raw",        "symbol-table", "symbol-table-raw", "name-tree",
    "name-tree-raw",    "resolve-tree",    "resolve-tree-raw",      "flatten-tree", "flatten-tree-raw", "cfg",
    "cfg-raw",          "autogen",         "document-symbols"};

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
        auto filename = uriToFilePath(config, diagnosticParams->uri);
        {
            INFO(fmt::format("Diagnostic URI is not a test file URI: {}", diagnosticParams->uri));
            CHECK_NE(testFileUris.end(), testFileUris.find(filename));
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
        return fmt::format("{}", fmt::map_join(symbols.begin(), symbols.end(), ", ",
                                               [](const auto &sym) -> string { return sym->toJSON(true); }));
    }
}

void testQuickFixCodeActions(LSPWrapper &lspWrapper, Expectations &test, UnorderedSet<string> &filenames,
                             vector<shared_ptr<RangeAssertion>> &assertions, UnorderedMap<string, string> &testFileUris,
                             int &nextId) {
    UnorderedMap<string, vector<shared_ptr<ApplyCodeActionAssertion>>> applyCodeActionAssertionsByFilename;
    for (auto &assertion : assertions) {
        if (auto applyCodeActionAssertion = dynamic_pointer_cast<ApplyCodeActionAssertion>(assertion)) {
            applyCodeActionAssertionsByFilename[applyCodeActionAssertion->filename].push_back(applyCodeActionAssertion);
        }
    }

    bool exhaustiveApplyCodeAction =
        BooleanPropertyAssertion::getValue("exhaustive-apply-code-action", assertions).value_or(false);

    if (applyCodeActionAssertionsByFilename.empty() && !exhaustiveApplyCodeAction) {
        return;
    }

    auto errors = RangeAssertion::getErrorAssertions(assertions);
    UnorderedMap<string, std::vector<std::shared_ptr<RangeAssertion>>> errorsByFilename;
    for (auto &error : errors) {
        errorsByFilename[error->filename].emplace_back(error);
    }

    for (auto &filename : filenames) {
        auto applyCodeActionAssertions = applyCodeActionAssertionsByFilename[filename];

        // Request code actions for each of this file's error.
        for (auto &error : errorsByFilename[filename]) {
            vector<unique_ptr<Diagnostic>> diagnostics;
            auto fileUri = testFileUris[filename];
            // Unfortunately there's no simpler way to copy the range (yet).
            auto params =
                make_unique<CodeActionParams>(make_unique<TextDocumentIdentifier>(fileUri), error->range->copy(),
                                              make_unique<CodeActionContext>(move(diagnostics)));
            auto req = make_unique<RequestMessage>("2.0", nextId++, LSPMethod::TextDocumentCodeAction, move(params));
            auto responses = getLSPResponsesFor(lspWrapper, make_unique<LSPMessage>(move(req)));
            {
                INFO("Did not receive exactly one response for a codeAction request.");
                CHECK_EQ(responses.size(), 1);
            }
            if (responses.size() != 1) {
                continue;
            }

            auto &msg = responses.at(0);
            CHECK(msg->isResponse());
            if (!msg->isResponse()) {
                continue;
            }

            auto &response = msg->asResponse();
            REQUIRE_MESSAGE(response.result, "Code action request returned error: " << msg->toJSON());
            auto &receivedCodeActionResponse =
                get<variant<JSONNullObject, vector<unique_ptr<CodeAction>>>>(*response.result);
            CHECK_FALSE(get_if<JSONNullObject>(&receivedCodeActionResponse));
            if (get_if<JSONNullObject>(&receivedCodeActionResponse)) {
                continue;
            }

            UnorderedMap<string, unique_ptr<CodeAction>> receivedCodeActionsByTitle;
            auto &receivedCodeActions = get<vector<unique_ptr<CodeAction>>>(receivedCodeActionResponse);
            unique_ptr<CodeAction> sourceLevelCodeAction;
            // One code action should be a 'source' level code action. Remove it.
            if (!receivedCodeActions.empty()) {
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

            u4 receivedCodeActionsCount = receivedCodeActionsByTitle.size();
            vector<shared_ptr<ApplyCodeActionAssertion>> matchedCodeActionAssertions;

            // Test code action assertions matching the range of this error.
            auto it = applyCodeActionAssertions.begin();
            while (it != applyCodeActionAssertions.end()) {
                auto codeActionAssertion = it->get();
                if (!(error->range->start->cmp(*codeActionAssertion->range->start) <= 0 &&
                      error->range->end->cmp(*codeActionAssertion->range->end) >= 0)) {
                    ++it;
                    continue;
                }

                // Ensure we received a code action matching the assertion.
                auto it2 = receivedCodeActionsByTitle.find(codeActionAssertion->title);
                {
                    INFO(fmt::format("Did not receive code action matching assertion `{}` for error `{}`...",
                                     codeActionAssertion->toString(), error->toString()));
                    CHECK_NE(it2, receivedCodeActionsByTitle.end());
                }

                // Ensure that the received code action applies correctly.
                if (it2 != receivedCodeActionsByTitle.end()) {
                    auto codeAction = move(it2->second);
                    codeActionAssertion->check(test.sourceFileContents, lspWrapper, *codeAction.get());

                    // Some bookkeeping to make surfacing errors re. extra/insufficient
                    // apply-code-action annotations easier.
                    receivedCodeActionsByTitle.erase(it2);
                    matchedCodeActionAssertions.emplace_back(*it);
                    it = applyCodeActionAssertions.erase(it);
                } else {
                    ++it;
                }
            }

            if (exhaustiveApplyCodeAction) {
                if (matchedCodeActionAssertions.size() > receivedCodeActionsCount) {
                    FAIL_CHECK(fmt::format(
                        "Found apply-code-action assertions without "
                        "corresponding code actions from the server:\n{}",
                        fmt::map_join(applyCodeActionAssertions.begin(), applyCodeActionAssertions.end(), ", ",
                                      [](const auto &assertion) -> string { return assertion->toString(); })));
                } else if (matchedCodeActionAssertions.size() < receivedCodeActionsCount) {
                    FAIL_CHECK(fmt::format(
                        "Received code actions without corresponding apply-code-action assertions:\n{}",
                        fmt::map_join(receivedCodeActionsByTitle.begin(), receivedCodeActionsByTitle.end(), "\n",
                                      [](const auto &action) -> string { return action.second->toJSON(); })));
                }
            }
        }

        // We've already removed any code action assertions that matches a received code action assertion.
        // Any remaining are therefore extraneous.
        INFO(fmt::format("Found extraneous apply-code-action assertions:\n{}",
                         fmt::map_join(applyCodeActionAssertions.begin(), applyCodeActionAssertions.end(), "\n",
                                       [](const auto &assertion) -> string { return assertion->toString(); })));
        CHECK_EQ(applyCodeActionAssertions.size(), 0);
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
    auto expected = LSPMessage::fromClient(FileOps::read(expectedSymbolsPath.c_str()));
    auto &expectedResp = expected->asResponse();
    auto &expectedSymbolResponse =
        get<variant<JSONNullObject, vector<unique_ptr<DocumentSymbol>>>>(*expectedResp.result);

    // Simple string comparison, just like other *.exp files.
    INFO("Mismatch on: " << expectedSymbolsPath);
    CHECK_EQ(documentSymbolsToString(expectedSymbolResponse), documentSymbolsToString(receivedSymbolResponse));
}

TEST_CASE("LSPTest") {
    /** The path to the test Ruby files on disk */
    UnorderedSet<std::string> filenames;
    std::unique_ptr<LSPWrapper> lspWrapper;

    /** All test assertions ordered by (filename, range, message). */
    std::vector<std::shared_ptr<RangeAssertion>> assertions;

    /** Test expectations. Stored here for convenience. */
    Expectations test;

    /** The next ID to use when sending an LSP message. */
    int nextId = 0;

    for (auto &sourceFile : test.sourceFiles) {
        filenames.insert(test.folder + sourceFile);
    }

    // Initialize lspWrapper.
    {
        shared_ptr<realmain::options::Options> opts = make_shared<realmain::options::Options>();
        opts->noStdlib = BooleanPropertyAssertion::getValue("no-stdlib", assertions).value_or(false);
        lspWrapper = SingleThreadedLSPWrapper::create("", move(opts));
        lspWrapper->enableAllExperimentalFeatures();
    }

    if (test.expectations.find("autogen") != test.expectations.end()) {
        // When autogen is enabled, skip Rewriter passes...
        lspWrapper->opts->skipRewriterPasses = true;
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

    // Perform initialize / initialized handshake.
    {
        string rootPath = fmt::format("/Users/{}/stripe/sorbet", std::getenv("USER"));
        string rootUri = fmt::format("file://{}", rootPath);
        auto sorbetInitOptions = make_unique<SorbetInitializationOptions>();
        sorbetInitOptions->enableTypecheckInfo = true;
        auto initializedResponses =
            initializeLSP(rootPath, rootUri, *lspWrapper, nextId, true, move(sorbetInitOptions));
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
            INFO("Should not receive any response to opening an empty file.");
            CHECK_EQ(0, countNonTestMessages(responses));
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
            vector<unique_ptr<LSPMessage>> updates;
            for (auto &filename : filenames) {
                auto textDocContents = test.sourceFileContents[filename]->source();
                updates.push_back(
                    makeChange(testFileUris[filename], string(textDocContents.begin(), textDocContents.end()), 2 + i));
            }
            auto responses = getLSPResponsesFor(*lspWrapper, move(updates));
            updateDiagnostics(config, testFileUris, responses, diagnostics);
            slowPathPassed = ErrorAssertion::checkAll(
                test.sourceFileContents, RangeAssertion::getErrorAssertions(assertions), diagnostics, errorPrefixes[i]);
            if (i == 2) {
                FAIL_CHECK("Note: To disable fast path tests, add `# disable-fast-path: true` to the file.");
            }
        }
    }

    for (auto &filename : filenames) {
        testDocumentSymbols(*lspWrapper, test, nextId, testFileUris[filename], filename);
    }
    testQuickFixCodeActions(*lspWrapper, test, filenames, assertions, testFileUris, nextId);

    // Usage and def assertions
    {
        // Sort by symbol.
        // symbol => [ (version => DefAssertion), (DefAssertion | UsageAssertion)[] ]
        // Note: Using a vector in pair since order matters; assertions are ordered by location, which
        // is used when comparing against LSP responses.
        UnorderedMap<string, pair<UnorderedMap<int, shared_ptr<DefAssertion>>, vector<shared_ptr<RangeAssertion>>>>
            defUsageMap;

        // symbol => [ TypeDefAssertion[], TypeAssertion[] ]
        //
        // (The first element of the pair is only ever TypeDefAssertions but we only ever care that they're
        // RangeAssertions, so rather than fiddle with up casting, we'll just make the whole vector RangeAssertions)
        UnorderedMap<string, pair<vector<shared_ptr<RangeAssertion>>, vector<shared_ptr<TypeAssertion>>>> typeDefMap;
        for (auto &assertion : assertions) {
            if (auto defAssertion = dynamic_pointer_cast<DefAssertion>(assertion)) {
                auto &entry = defUsageMap[defAssertion->symbol];
                auto &defMap = entry.first;
                INFO(fmt::format(
                    "Found multiple def comments for label `{}` version `{}`.\nPlease use unique labels and versions "
                    "for definition assertions. Note that these labels do not need to match the pointed-to "
                    "identifiers.\nFor example, the following is completely valid:\n foo = 3\n#^^^ def: bar 100",
                    defAssertion->symbol, defAssertion->version));
                CHECK_FALSE(defMap.contains(defAssertion->version));
                defMap[defAssertion->version] = defAssertion;
                entry.second.push_back(defAssertion);
            } else if (auto usageAssertion = dynamic_pointer_cast<UsageAssertion>(assertion)) {
                auto &entry = defUsageMap[usageAssertion->symbol];
                entry.second.push_back(usageAssertion);
            } else if (auto typeDefAssertion = dynamic_pointer_cast<TypeDefAssertion>(assertion)) {
                auto &[typeDefs, _typeAssertions] = typeDefMap[typeDefAssertion->symbol];
                typeDefs.push_back(typeDefAssertion);
            } else if (auto typeAssertion = dynamic_pointer_cast<TypeAssertion>(assertion)) {
                auto &[_typeDefs, typeAssertions] = typeDefMap[typeAssertion->symbol];
                typeAssertions.push_back(typeAssertion);
            }
        }

        // Check each def/usage assertion.
        for (auto &entry : defUsageMap) {
            auto &entryAssertions = entry.second.second;
            // Sort assertions in (filename, range) order
            fast_sort(entryAssertions,
                      [](const shared_ptr<RangeAssertion> &a, const shared_ptr<RangeAssertion> &b) -> bool {
                          return a->cmp(*b) < 0;
                      });

            auto &defAssertions = entry.second.first;
            // Shouldn't be possible to have an entry with 0 assertions, but explicitly check anyway.
            CHECK_GE(entryAssertions.size(), 1);

            for (auto &assertion : entryAssertions) {
                string_view symbol;
                int version = -1;
                if (auto defAssertion = dynamic_pointer_cast<DefAssertion>(assertion)) {
                    version = defAssertion->version;
                    symbol = defAssertion->symbol;
                } else if (auto usageAssertion = dynamic_pointer_cast<UsageAssertion>(assertion)) {
                    version = usageAssertion->version;
                    symbol = usageAssertion->symbol;
                }
                auto entry = defAssertions.find(version);
                if (entry != defAssertions.end()) {
                    auto &def = entry->second;
                    auto queryLoc = assertion->getLocation(config);
                    // Check that a definition request at this location returns def.
                    def->check(test.sourceFileContents, *lspWrapper, nextId, *queryLoc);
                    // Check that a reference request at this location returns entryAssertions.
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
                } else {
                    FAIL_CHECK(fmt::format(
                        "Found usage comment for label {0} version {1} without matching def comment. Please add a `# "
                        "^^ def: {0} {1}` assertion that points to the definition of the pointed-to thing being used.",
                        symbol, version));
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
    }

    // Hover assertions
    HoverAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

    // Completion assertions
    CompletionAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);
    ApplyCompletionAssertion::checkAll(assertions, test.sourceFileContents, *lspWrapper, nextId);

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
            auto &updates = test.sourceLSPFileUpdates[version];
            vector<unique_ptr<LSPMessage>> lspUpdates;
            UnorderedMap<std::string, std::shared_ptr<core::File>> updatesAndContents;

            for (auto &update : updates) {
                auto originalFile = test.folder + update.first;
                auto updateFile = test.folder + update.second;
                auto fileContents = FileOps::read(updateFile);
                lspUpdates.push_back(makeChange(testFileUris[originalFile], fileContents, baseVersion + version));
                updatesAndContents[originalFile] =
                    make_shared<core::File>(string(originalFile), move(fileContents), core::File::Type::Normal);
            }
            auto assertions = RangeAssertion::parseAssertions(updatesAndContents);
            auto assertFastPath = FastPathAssertion::get(assertions);
            auto assertSlowPath = BooleanPropertyAssertion::getValue("assert-slow-path", assertions);
            auto responses = getLSPResponsesFor(*lspWrapper, move(lspUpdates));
            bool foundTypecheckRunInfo = false;

            for (auto &r : responses) {
                if (r->isNotification()) {
                    if (r->method() == LSPMethod::SorbetTypecheckRunInfo) {
                        auto &params = get<unique_ptr<SorbetTypecheckRunInfo>>(r->asNotification().params);
                        // Ignore started messages. Note that cancelation messages cannot occur in test_corpus since
                        // test_corpus only runs LSP in single-threaded mode.
                        if (params->status == SorbetTypecheckRunStatus::Ended) {
                            foundTypecheckRunInfo = true;
                            if (assertSlowPath.value_or(false)) {
                                INFO(errorPrefix << "Expected Sorbet to take slow path, but it took the fast path.");
                                CHECK_EQ(params->fastPath, false);
                            }
                            if (assertFastPath.has_value()) {
                                (*assertFastPath)->check(*params, test.folder, version, errorPrefix);
                            }
                        }
                    } else if (r->method() != LSPMethod::TextDocumentPublishDiagnostics) {
                        FAIL_CHECK(errorPrefix
                                   << fmt::format("Unexpected message response to file update of type {}:\n{}",
                                                  convertLSPMethodToString(r->method()), r->toJSON()));
                    }
                } else {
                    FAIL_CHECK(errorPrefix
                               << fmt::format("Unexpected message response to file update:\n{}", r->toJSON()));
                }
            }

            if (!foundTypecheckRunInfo) {
                FAIL_CHECK(errorPrefix << "Sorbet did not send expected typechecking metadata.");
            }

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

    doctest::Context context;
    return context.run();
}
