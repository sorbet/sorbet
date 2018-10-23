#include "main/lsp/lsp.h"
#include "absl/algorithm/container.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::LSPLoop(unique_ptr<core::GlobalState> gs, const options::Options &opts, shared_ptr<spd::logger> &logger,
                 WorkerPool &workers)
    : initialGS(move(gs)), opts(opts), logger(logger), workers(workers) {
    errorQueue = dynamic_pointer_cast<realmain::ConcurrentErrorQueue>(initialGS->errorQueue);
    ENFORCE(errorQueue, "LSPLoop got an unexpected error queue");
}

namespace {
class LSPQuerrySetup {
    core::GlobalState &gs;

public:
    LSPQuerrySetup(core::GlobalState &gs, core::Loc loc, core::SymbolRef symbol) : gs(gs) {
        ENFORCE(!gs.lspInfoQueryLoc.exists());
        ENFORCE(!gs.lspQuerySymbol.exists());
        ENFORCE(loc.exists() || symbol.exists());
        gs.lspInfoQueryLoc = loc;
        gs.lspQuerySymbol = symbol;
    }
    ~LSPQuerrySetup() {
        gs.lspInfoQueryLoc = core::Loc::none();
        gs.lspQuerySymbol = core::Symbols::noSymbol();
    }
};

} // namespace

bool LSPLoop::setupLSPQueryByLoc(rapidjson::Document &d, const LSPMethod &forMethod, bool errorIfFileIsUntyped) {
    auto uri = string_view(d["params"]["textDocument"]["uri"].GetString(),
                           d["params"]["textDocument"]["uri"].GetStringLength());

    auto fref = uri2FileRef(uri);
    if (!fref.exists()) {
        sendError(d, (int)LSPErrorCodes::InvalidParams,
                  fmt::format("Did not find file at uri {} in {}", uri, forMethod.name));
        return false;
    }

    if (errorIfFileIsUntyped && fref.data(*finalGs).sigil == core::StrictLevel::Stripe) {
        sendError(d, (int)LSPErrorCodes::InvalidParams, "This feature only works correctly on typed ruby files.");
        sendShowMessageNotification(
            1, "This feature only works correctly on typed ruby files. Results you see may be heuristic results.");
        return false;
    }
    auto loc = lspPos2Loc(fref, d, *finalGs);
    if (!loc) {
        sendError(d, (int)LSPErrorCodes::InvalidParams,
                  fmt::format("Did not find location at uri {} in {}", uri, forMethod.name));
        return false;
    }

    {
        LSPQuerrySetup setup1(*initialGS, *loc, core::Symbols::noSymbol());
        LSPQuerrySetup setup2(*finalGs, *loc, core::Symbols::noSymbol());
        vector<shared_ptr<core::File>> files;
        files.emplace_back(make_shared<core::File>((move(fref.data(*finalGs)))));
        tryFastPath(files);
    }

    return true;
}
bool LSPLoop::setupLSPQueryBySymbol(core::SymbolRef sym, const LSPMethod &forMethod) {
    ENFORCE(sym.exists());
    vector<shared_ptr<core::File>> files;
    {
        LSPQuerrySetup setup(*finalGs, core::Loc::none(), sym);
        tryFastPath(files, true);
    }
    // this function currently always returns true, but we're keeping API symmetric to setupLSPQueryByLoc.
    return true;
}

bool silenceError(core::ErrorClass what) {
    if (what == core::errors::Namer::RedefinitionOfMethod ||
        what == core::errors::Resolver::DuplicateVariableDeclaration ||
        what == core::errors::Resolver::RedefinitionOfParents) {
        return true;
    }
    return false;
}

void LSPLoop::drainErrors() {
    for (auto &e : errorQueue->drainAllErrors()) {
        if (e->isSilenced || silenceError(e->what)) {
            continue;
        }
        auto file = e->loc.file();
        errorsAccumulated[file].emplace_back(move(e));

        if (!updatedErrors.empty() && updatedErrors.back() == file) {
            continue;
        }
        updatedErrors.emplace_back(file);
    }
    auto iter = errorsAccumulated.begin();
    for (; iter != errorsAccumulated.end();) {
        if (iter->first.exists() && iter->first.data(*initialGS).sourceType == core::File::TombStone) {
            iter = errorsAccumulated.erase(iter);
        } else {
            ++iter;
        }
    }
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

void LSPLoop::pushErrors() {
    drainErrors();

    // Dedup updates
    absl::c_sort(updatedErrors);
    updatedErrors.erase(unique(updatedErrors.begin(), updatedErrors.end()), updatedErrors.end());

    for (auto file : updatedErrors) {
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
                        diagnostic.AddMember("message", e->formatted, alloc);

                        typecase(e.get(), [&](core::ComplexError *ce) {
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
    updatedErrors.clear();
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
