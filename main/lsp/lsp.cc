#include "main/lsp/lsp.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::LSPLoop(unique_ptr<core::GlobalState> gs, const options::Options &opts, shared_ptr<spd::logger> &logger,
                 WorkerPool &workers, std::istream &input, std::ostream &output, bool typecheckTestFiles,
                 bool skipConfigatron, bool disableFastPath)
    : initialGS(std::move(gs)), opts(opts), logger(logger), workers(workers), istream(input), ostream(output),
      typecheckTestFiles(typecheckTestFiles), skipConfigatron(skipConfigatron), disableFastPath(disableFastPath) {
    errorQueue = dynamic_pointer_cast<core::ErrorQueue>(initialGS->errorQueue);
    ENFORCE(errorQueue, "LSPLoop got an unexpected error queue");
    ENFORCE(errorQueue->ignoreFlushes,
            "LSPLoop's error queue is not ignoring flushes, which will prevent LSP from sending diagnostics");
}

namespace {
class LSPQuerrySetup {
    std::unique_ptr<core::GlobalState> &gs;

public:
    LSPQuerrySetup(std::unique_ptr<core::GlobalState> &gs, core::Loc loc, core::SymbolRef symbol) : gs(gs) {
        // Might return 'true' if GS was copied with flags set before they were unset.
        // ENFORCE(!gs.lspInfoQueryLoc.exists());
        // ENFORCE(!gs.lspQuerySymbol.exists());
        ENFORCE(loc.exists() || symbol.exists());
        gs->lspInfoQueryLoc = loc;
        gs->lspQuerySymbol = symbol;
    }
    ~LSPQuerrySetup() {
        gs->lspInfoQueryLoc = core::Loc::none();
        gs->lspQuerySymbol = core::Symbols::noSymbol();
    }
};

} // namespace

optional<LSPLoop::TypecheckRun> LSPLoop::setupLSPQueryByLoc(rapidjson::Document &d, const LSPMethod &forMethod,
                                                            bool errorIfFileIsUntyped) {
    auto uri = string_view(d["params"]["textDocument"]["uri"].GetString(),
                           d["params"]["textDocument"]["uri"].GetStringLength());

    auto fref = uri2FileRef(uri);
    if (!fref.exists()) {
        sendError(d, (int)LSPErrorCodes::InvalidParams,
                  fmt::format("Did not find file at uri {} in {}", uri, forMethod.name));
        return nullopt;
    }

    if (errorIfFileIsUntyped && fref.data(*finalGs).sigil == core::StrictLevel::Stripe) {
        sendError(d, (int)LSPErrorCodes::InvalidParams, "This feature only works correctly on typed ruby files.");
        sendShowMessageNotification(
            1, "This feature only works correctly on typed ruby files. Results you see may be heuristic results.");
        return nullopt;
    }
    auto loc = lspPos2Loc(fref, d, *finalGs);
    if (!loc) {
        sendError(d, (int)LSPErrorCodes::InvalidParams,
                  fmt::format("Did not find location at uri {} in {}", uri, forMethod.name));
        return nullopt;
    }

    LSPQuerrySetup setup1(initialGS, *loc.get(), core::Symbols::noSymbol());
    LSPQuerrySetup setup2(finalGs, *loc.get(), core::Symbols::noSymbol());
    vector<shared_ptr<core::File>> files;
    files.emplace_back(make_shared<core::File>((std::move(fref.data(*finalGs)))));
    return tryFastPath(files);
}
optional<LSPLoop::TypecheckRun> LSPLoop::setupLSPQueryBySymbol(core::SymbolRef sym, const LSPMethod &forMethod) {
    ENFORCE(sym.exists());
    vector<shared_ptr<core::File>> files;
    {
        LSPQuerrySetup setup1(initialGS, core::Loc::none(), sym);
        LSPQuerrySetup setup2(finalGs, core::Loc::none(), sym);
        return tryFastPath(files, true);
    }
    // this function currently always returns optional that is set, but we're keeping API symmetric to
    // setupLSPQueryByLoc.
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

bool LSPLoop::ensureInitialized(LSPMethod forMethod, rapidjson::Document &d) {
    if (finalGs) {
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

void LSPLoop::pushDiagnostics(TypecheckRun run) {
    const auto &filesTypechecked = run.filesTypechecked;
    std::vector<core::FileRef> errorFilesInNewRun;
    UnorderedMap<core::FileRef, std::vector<std::unique_ptr<core::Error>>> errorsAccumulated;

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

    std::vector<core::FileRef> filesToUpdateErrorListFor = errorFilesInNewRun;

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
    filesToUpdateErrorListFor.erase(std::unique(filesToUpdateErrorListFor.begin(), filesToUpdateErrorListFor.end()),
                                    filesToUpdateErrorListFor.end());

    fast_sort(errorFilesInNewRun);
    errorFilesInNewRun.erase(std::unique(errorFilesInNewRun.begin(), errorFilesInNewRun.end()),
                             errorFilesInNewRun.end());

    this->filesThatHaveErrors = errorFilesInNewRun;

    for (auto file : filesToUpdateErrorListFor) {
        if (file.exists()) {
            rapidjson::Value publishDiagnosticsParams;

            /**
             * interface PublishDiagnosticsParams {
             *      uri: DocumentUri; // The URI for which diagnostic information is reported.
             *      diagnostics: Diagnostic[]; // An array of diagnostic information items.
             * }
             **/
            /** interface Diagnostic {
             *      range: Range; // The range at which the message applies.
             *      severity?: number; // The diagnostic's severity.
             *      code?: number | string; // The diagnostic's code
             *      source?: string; // A human-readable string describing the source of this diagnostic, e.g.
             *                       // 'typescript' or 'super lint'.
             *      message: string; // The diagnostic's message. relatedInformation?:
             *      DiagnosticRelatedInformation[]; // An array of related diagnostic information
             * }
             **/

            publishDiagnosticsParams.SetObject();
            { // uri
                string uriStr;
                if (file.data(*finalGs).sourceType == core::File::Type::Payload) {
                    uriStr = string(file.data(*finalGs).path());
                } else {
                    uriStr = fmt::format("{}/{}", rootUri, file.data(*finalGs).path());
                }
                publishDiagnosticsParams.AddMember("uri", uriStr, alloc);
            }

            {
                // diagnostics
                rapidjson::Value diagnostics;
                diagnostics.SetArray();
                if (errorsAccumulated.find(file) != errorsAccumulated.end()) {
                    for (auto &e : errorsAccumulated[file]) {
                        rapidjson::Value diagnostic;
                        diagnostic.SetObject();

                        diagnostic.AddMember("range", loc2Range(e->loc), alloc);
                        diagnostic.AddMember("code", e->what.code, alloc);
                        diagnostic.AddMember("message", e->header, alloc);

                        typecase(e.get(), [&](core::Error *ce) {
                            rapidjson::Value relatedInformation;
                            relatedInformation.SetArray();
                            for (auto &section : ce->sections) {
                                string sectionHeader = section.header;

                                for (auto &errorLine : section.messages) {
                                    rapidjson::Value relatedInfo;
                                    relatedInfo.SetObject();
                                    relatedInfo.AddMember("location", loc2Location(errorLine.loc), alloc);

                                    string relatedInfoMessage;
                                    if (errorLine.formattedMessage.length() > 0) {
                                        relatedInfoMessage = errorLine.formattedMessage;
                                    } else {
                                        relatedInfoMessage = sectionHeader;
                                    }
                                    relatedInfo.AddMember("message", relatedInfoMessage, alloc);
                                    relatedInformation.PushBack(relatedInfo, alloc);
                                }
                            }
                            diagnostic.AddMember("relatedInformation", relatedInformation, alloc);
                        });
                        diagnostics.PushBack(diagnostic, alloc);
                    }
                }
                publishDiagnosticsParams.AddMember("diagnostics", diagnostics, alloc);
            }

            sendNotification(LSPMethod::PushDiagnostics(), publishDiagnosticsParams);
        }
    }
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
