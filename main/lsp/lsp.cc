#include "main/lsp/lsp.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::LSPLoop(unique_ptr<core::GlobalState> gs, const options::Options &opts, shared_ptr<spd::logger> &logger,
                 WorkerPool &workers, istream &inputStream, std::ostream &outputStream, bool typecheckTestFiles,
                 bool skipConfigatron, bool disableFastPath)
    : initialGS(std::move(gs)), opts(opts), logger(logger), workers(workers), inputStream(inputStream),
      outputStream(outputStream), typecheckTestFiles(typecheckTestFiles), skipConfigatron(skipConfigatron),
      disableFastPath(disableFastPath) {
    errorQueue = dynamic_pointer_cast<core::ErrorQueue>(initialGS->errorQueue);
    ENFORCE(errorQueue, "LSPLoop got an unexpected error queue");
    ENFORCE(errorQueue->ignoreFlushes,
            "LSPLoop's error queue is not ignoring flushes, which will prevent LSP from sending diagnostics");
}

LSPLoop::TypecheckRun LSPLoop::runLSPQuery(unique_ptr<core::GlobalState> gs, core::Loc loc, core::SymbolRef symbol,
                                           vector<shared_ptr<core::File>> &changedFiles, bool allFiles) {
    ENFORCE(!gs->lspInfoQueryLoc.exists());
    ENFORCE(!initialGS->lspInfoQueryLoc.exists());
    ENFORCE(!gs->lspQuerySymbol.exists());
    ENFORCE(!initialGS->lspQuerySymbol.exists());
    ENFORCE(loc.exists() || symbol.exists());
    initialGS->lspInfoQueryLoc = gs->lspInfoQueryLoc = loc;
    initialGS->lspQuerySymbol = gs->lspQuerySymbol = symbol;

    // TODO(jvilk): If this throws, then we'll want to unset `lspInfoQueryLoc` and `lspQuerySymbol` on `initialGS`.
    // If throwing is common, then we need some way to *not* throw away `gs`.
    auto rv = tryFastPath(move(gs), changedFiles, allFiles);
    rv.gs->lspInfoQueryLoc = initialGS->lspInfoQueryLoc = core::Loc::none();
    rv.gs->lspQuerySymbol = initialGS->lspQuerySymbol = core::Symbols::noSymbol();
    return rv;
}

LSPLoop::TypecheckRun LSPLoop::setupLSPQueryByLoc(unique_ptr<core::GlobalState> gs, rapidjson::Document &d,
                                                  const LSPMethod &forMethod, bool errorIfFileIsUntyped) {
    auto uri = string_view(d["params"]["textDocument"]["uri"].GetString(),
                           d["params"]["textDocument"]["uri"].GetStringLength());

    auto fref = uri2FileRef(uri);
    if (!fref.exists()) {
        sendError(d, (int)LSPErrorCodes::InvalidParams,
                  fmt::format("Did not find file at uri {} in {}", uri, forMethod.name));
        return TypecheckRun{{}, {}, {}, move(gs)};
    }

    if (errorIfFileIsUntyped && fref.data(*gs).sigil == core::StrictLevel::Stripe) {
        sendError(d, (int)LSPErrorCodes::InvalidParams, "This feature only works correctly on typed ruby files.");
        sendShowMessageNotification(
            MessageType::Error, d,
            "This feature only works correctly on typed ruby files. Results you see may be heuristic results.");
        return TypecheckRun{{}, {}, {}, move(gs)};
    }
    auto loc = lspPos2Loc(fref, d, *gs);
    if (!loc) {
        sendError(d, (int)LSPErrorCodes::InvalidParams,
                  fmt::format("Did not find location at uri {} in {}", uri, forMethod.name));
        return TypecheckRun{{}, {}, {}, move(gs)};
    }

    vector<shared_ptr<core::File>> files;
    files.emplace_back(make_shared<core::File>(std::move(fref.data(*gs))));
    return runLSPQuery(move(gs), *loc.get(), core::Symbols::noSymbol(), files);
}
LSPLoop::TypecheckRun LSPLoop::setupLSPQueryBySymbol(unique_ptr<core::GlobalState> gs, core::SymbolRef sym,
                                                     const LSPMethod &forMethod) {
    ENFORCE(sym.exists());
    vector<shared_ptr<core::File>> files;
    // this function currently always returns optional that is set, but we're keeping API symmetric to
    // setupLSPQueryByLoc.
    return runLSPQuery(move(gs), core::Loc::none(), sym, files, true);
}

bool silenceError(bool disableFastPath, core::ErrorClass what) {
    if (disableFastPath) {
        // We only need to silence errors during the fast path.
        return false;
    }
    if (what == core::errors::Namer::RedefinitionOfMethod ||
        what == core::errors::Resolver::DuplicateVariableDeclaration ||
        what == core::errors::Resolver::RedefinitionOfParents) {
        return true;
    }
    return false;
}

bool LSPLoop::ensureInitialized(LSPMethod forMethod, rapidjson::Document &d,
                                const unique_ptr<core::GlobalState> &currentGs) {
    if (currentGs) {
        return true;
    }
    if (forMethod == LSPMethod::Initialize() || forMethod == LSPMethod::Initialized() ||
        forMethod == LSPMethod::Exit() || forMethod == LSPMethod::Shutdown()) {
        return true;
    }
    logger->error("Serving request before got an Initialize & Initialized handshake from IDE");
    if (!forMethod.isNotification) {
        sendError(d, (int)LSPErrorCodes::ServerNotInitialized,
                  "IDE did not initialize Sorbet correctly. No requests should be made before Initialize&Initialized "
                  "have been completed");
    }
    return false;
}

unique_ptr<core::GlobalState> LSPLoop::pushDiagnostics(rapidjson::Document &d, TypecheckRun run) {
    const core::GlobalState &gs = *run.gs;
    const auto &filesTypechecked = run.filesTypechecked;
    vector<core::FileRef> errorFilesInNewRun;
    UnorderedMap<core::FileRef, vector<std::unique_ptr<core::Error>>> errorsAccumulated;

    for (auto &e : run.errors) {
        if (e->isSilenced || silenceError(disableFastPath, e->what)) {
            continue;
        }
        auto file = e->loc.file();
        errorsAccumulated[file].emplace_back(std::move(e));
    }

    for (auto &accumulated : errorsAccumulated) {
        errorFilesInNewRun.push_back(accumulated.first);
    }

    vector<core::FileRef> filesToUpdateErrorListFor = errorFilesInNewRun;

    UnorderedSet<core::FileRef> filesTypecheckedAsSet;
    filesTypecheckedAsSet.insert(filesTypechecked.begin(), filesTypechecked.end());

    for (auto f : this->filesThatHaveErrors) {
        if (filesTypecheckedAsSet.find(f) != filesTypecheckedAsSet.end()) {
            // we've retypechecked this file. We can override the fact it has an error
            // thus, we will update the error list for this file on client
            filesToUpdateErrorListFor.push_back(f);
        } else {
            // we're not typecking this file, we need to remember that it had error
            errorFilesInNewRun.push_back(f);
        }
    }

    fast_sort(filesToUpdateErrorListFor);
    filesToUpdateErrorListFor.erase(unique(filesToUpdateErrorListFor.begin(), filesToUpdateErrorListFor.end()),
                                    filesToUpdateErrorListFor.end());

    fast_sort(errorFilesInNewRun);
    errorFilesInNewRun.erase(unique(errorFilesInNewRun.begin(), errorFilesInNewRun.end()), errorFilesInNewRun.end());

    this->filesThatHaveErrors = errorFilesInNewRun;

    for (auto file : filesToUpdateErrorListFor) {
        if (file.exists()) {
            string uri;
            { // uri
                if (file.data(gs).sourceType == core::File::Type::Payload) {
                    uri = string(file.data(gs).path());
                } else {
                    uri = fmt::format("{}/{}", rootUri, file.data(gs).path());
                }
            }

            vector<unique_ptr<Diagnostic>> diagnostics;
            {
                // diagnostics
                if (errorsAccumulated.find(file) != errorsAccumulated.end()) {
                    for (auto &e : errorsAccumulated[file]) {
                        auto diagnostic = make_unique<Diagnostic>(loc2Range(gs, e->loc), e->header);
                        diagnostic->code = e->what.code;

                        typecase(e.get(), [&](core::Error *ce) {
                            vector<unique_ptr<DiagnosticRelatedInformation>> relatedInformation;
                            for (auto &section : ce->sections) {
                                string sectionHeader = section.header;

                                for (auto &errorLine : section.messages) {
                                    string message;
                                    if (errorLine.formattedMessage.length() > 0) {
                                        message = errorLine.formattedMessage;
                                    } else {
                                        message = sectionHeader;
                                    }
                                    relatedInformation.push_back(make_unique<DiagnosticRelatedInformation>(
                                        loc2Location(gs, errorLine.loc), message));
                                }
                            }
                            diagnostic->relatedInformation = move(relatedInformation);
                        });
                        diagnostics.push_back(move(diagnostic));
                    }
                }
            }

            sendNotification(LSPMethod::PushDiagnostics(), d, PublishDiagnosticsParams(uri, move(diagnostics)));
        }
    }
    return move(run.gs);
}

const vector<LSPMethod> LSPMethod::ALL_METHODS{CancelRequest(),
                                               Initialize(),
                                               Shutdown(),
                                               Exit(),
                                               RegisterCapability(),
                                               UnRegisterCapability(),
                                               DidChangeWatchedFiles(),
                                               PushDiagnostics(),
                                               TextDocumentDidOpen(),
                                               TextDocumentDidChange(),
                                               TextDocumentDocumentSymbol(),
                                               TextDocumentDefinition(),
                                               TextDocumentHover(),
                                               TextDocumentCompletion(),
                                               TextDocumentRefernces(),
                                               TextDocumentSignatureHelp(),
                                               WorkspaceSymbols(),
                                               CancelRequest(),
                                               Pause(),
                                               Resume()};

const LSPMethod LSPMethod::getByName(string_view name) {
    for (auto &candidate : ALL_METHODS) {
        if (candidate.name == name) {
            return candidate;
        }
    }
    return LSPMethod{string(name), true, LSPMethod::Kind::ClientInitiated, false};
}

} // namespace sorbet::realmain::lsp
