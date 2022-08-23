#include "main/lsp/LSPTask.h"
#include "absl/strings/match.h"
#include "absl/synchronization/notification.h"
#include "common/sort.h"
#include "core/FileHash.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

namespace sorbet::realmain::lsp {
using namespace std;

namespace {

// Enum used for tracking success/failure for requests. Basically only used to increment the
// respective counter
enum class ResponseMessageStatus {
    // Unknown status (will not increment any counter)
    Unknown = 0,
    // Request errored, and returned a RPC error (ResponseMessage::error)
    Errored,
    // Request succeeded, but return a known "empty" result for that response type.
    // (This could be a null result, an empty list, etc. depending on the response type)
    EmptyResult,
    // Request succeeded, and returned a meaningful result.
    Succeeded,
};

ResponseMessageStatus statusForResponse(const ResponseMessage &response) {
    if (response.error.has_value()) {
        ENFORCE(response.error.value() != nullptr);
        return ResponseMessageStatus::Errored;
    } else if (response.result.has_value()) {
        return visit(
            [](auto &&res) -> ResponseMessageStatus {
                using T = decay_t<decltype(res)>;
                // Note: Many of these are Unknown just out of laziness / not needing or wanting to
                // track them. Feel free to implement any case here more sensibly.
                if constexpr (is_same_v<T, unique_ptr<SorbetCounters>>) {
                    // __GETCOUNTERS__
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, unique_ptr<InitializeResult>>) {
                    // initialize
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, JSONNullObject>) {
                    // shutdown
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, variant<JSONNullObject, vector<unique_ptr<DocumentHighlight>>>>) {
                    // textDocument/documentHighlight
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, variant<JSONNullObject, vector<unique_ptr<DocumentSymbol>>>>) {
                    // textDocument/documentSymbol
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, variant<JSONNullObject, vector<unique_ptr<Location>>>>) {
                    // textDocument/definition, textDocument/typeDefinition, textDocument/references, and
                    // textDocument/implementation
                    if (const auto *locationsPtr = get_if<vector<unique_ptr<Location>>>(&res)) {
                        return locationsPtr->empty() ? ResponseMessageStatus::EmptyResult
                                                     : ResponseMessageStatus::Succeeded;
                    } else {
                        return ResponseMessageStatus::EmptyResult;
                    }
                } else if constexpr (is_same_v<T, variant<JSONNullObject, unique_ptr<Hover>>>) {
                    // textDocument/hover
                    if (const auto *hoverPtr = get_if<unique_ptr<Hover>>(&res)) {
                        auto &hover = *hoverPtr;
                        return hover->contents->value.empty() ? ResponseMessageStatus::EmptyResult
                                                              : ResponseMessageStatus::Succeeded;
                    } else {
                        return ResponseMessageStatus::EmptyResult;
                    }
                } else if constexpr (is_same_v<T, unique_ptr<CompletionList>>) {
                    // textDocument/completion
                    return res->items.empty() ? ResponseMessageStatus::EmptyResult : ResponseMessageStatus::Succeeded;
                } else if constexpr (is_same_v<T, variant<JSONNullObject, unique_ptr<PrepareRenameResult>>>) {
                    // textDocument/prepareRename
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, variant<JSONNullObject, unique_ptr<WorkspaceEdit>>>) {
                    // textDocument/rename
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, variant<JSONNullObject, unique_ptr<SignatureHelp>>>) {
                    // textDocument/signatureHelp
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, variant<JSONNullObject, vector<unique_ptr<TextEdit>>>>) {
                    // textDocument/formatting
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, variant<JSONNullObject, vector<unique_ptr<CodeAction>>>>) {
                    // textDocument/codeAction
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, optional<unique_ptr<CodeAction>>>) {
                    //  codeAction/resolve
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, variant<JSONNullObject, vector<unique_ptr<SymbolInformation>>>>) {
                    // workspace/symbol
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, unique_ptr<SorbetErrorParams>>) {
                    // sorbet/error
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, unique_ptr<SorbetFenceParams>>) {
                    // sorbet/fence
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, unique_ptr<TextDocumentItem>>) {
                    // sorbet/readFile
                    return ResponseMessageStatus::Unknown;
                } else if constexpr (is_same_v<T, variant<JSONNullObject, std::unique_ptr<SymbolInformation>>>) {
                    // sorbet/showSymbol
                    return holds_alternative<JSONNullObject>(res) ? ResponseMessageStatus::EmptyResult
                                                                  : ResponseMessageStatus::Succeeded;
                } else {
                    static_assert(always_false_v<T>, "non-exhaustive visitor!");
                }
            },
            response.result.value());
    } else {
        ENFORCE(false, "ResponseMessage should have had eithe error or result set");
        return ResponseMessageStatus::Unknown;
    }
}

} // namespace

LSPTask::LSPTask(const LSPConfiguration &config, LSPMethod method) : config(config), method(method) {}

LSPTask::~LSPTask() {
    if (!latencyTimer) {
        return;
    }
    latencyTimer->setTag("method", methodString());
}

ConstExprStr LSPTask::methodString() const {
    switch (method) {
        case LSPMethod::$CancelRequest:
            return "cancelRequest";
        case LSPMethod::Exit:
            return "exit";
        case LSPMethod::PAUSE:
            return "PAUSE";
        case LSPMethod::RESUME:
            return "RESUME";
        case LSPMethod::GETCOUNTERS:
            return "GETCOUNTERS";
        case LSPMethod::Shutdown:
            return "shutdown";
        case LSPMethod::SorbetError:
            return "sorbet.error";
        case LSPMethod::SorbetFence:
            return "sorbet.fence";
        case LSPMethod::SorbetShowOperation:
        case LSPMethod::SorbetTypecheckRunInfo:
        case LSPMethod::TextDocumentPublishDiagnostics:
        case LSPMethod::WindowShowMessage:
            // Sorbet sends these to the client; the server shouldn't receive them.
            return "other";
        case LSPMethod::Initialize:
            return "initialize";
        case LSPMethod::Initialized:
            return "initialized";
        case LSPMethod::SorbetReadFile:
            return "sorbet.readFile";
        case LSPMethod::SorbetIndexerInitialization:
            return "sorbet.indexerInitialization";
        case LSPMethod::SorbetShowSymbol:
            return "sorbet.showSymbol";
        case LSPMethod::SorbetWatchmanFileChange:
            return "sorbet.watchmanFileChange";
        case LSPMethod::SorbetWorkspaceEdit:
        case LSPMethod::TextDocumentDidChange:
        case LSPMethod::TextDocumentDidClose:
        case LSPMethod::TextDocumentDidOpen:
            return "sorbet.workspaceEdit";
        case LSPMethod::TextDocumentCodeAction:
            return "textDocument.codeAction";
        case LSPMethod::TextDocumentCompletion:
            return LSP_COMPLETION_METRICS_PREFIX;
        case LSPMethod::TextDocumentDefinition:
            return "textDocument.definition";
        case LSPMethod::TextDocumentDocumentHighlight:
            return "textDocument.documentHighlight";
        case LSPMethod::TextDocumentDocumentSymbol:
            return "textDocument.documentSymbol";
        case LSPMethod::TextDocumentFormatting:
            return "textDocument.formatting";
        case LSPMethod::TextDocumentHover:
            return "textDocument.hover";
        case LSPMethod::TextDocumentPrepareRename:
            return "textDocument.prepareRename";
        case LSPMethod::TextDocumentReferences:
            return "textDocument.references";
        case LSPMethod::TextDocumentRename:
            return "textDocument.rename";
        case LSPMethod::TextDocumentSignatureHelp:
            return "textDocument.signatureHelp";
        case LSPMethod::TextDocumentTypeDefinition:
            return "textDocument.typeDefinition";
        case LSPMethod::WorkspaceSymbol:
            return "workspace.symbol";
        case LSPMethod::TextDocumentImplementation:
            return "textDocument.implementation";
        case LSPMethod::CodeActionResolve:
            return "codeAction.resolve";
    }
}

LSPTask::Phase LSPTask::finalPhase() const {
    return Phase::RUN;
}

void LSPTask::preprocess(LSPPreprocessor &preprocessor) {}

void LSPTask::index(LSPIndexer &indexer) {}

LSPRequestTask::LSPRequestTask(const LSPConfiguration &config, MessageId id, LSPMethod method)
    : LSPTask(config, method), id(move(id)) {}

void LSPRequestTask::run(LSPTypecheckerInterface &typechecker) {
    auto response = runRequest(typechecker);
    ENFORCE(response != nullptr);

    switch (statusForResponse(*response)) {
        case ResponseMessageStatus::Unknown:
            break;
        case ResponseMessageStatus::Errored:
            prodCategoryCounterInc("lsp.messages.run.errored", methodString());
            break;
        case ResponseMessageStatus::EmptyResult:
            prodCategoryCounterInc("lsp.messages.run.emptyresult", methodString());
            break;
        case ResponseMessageStatus::Succeeded:
            prodCategoryCounterInc("lsp.messages.run.succeeded", methodString());
            break;
    }

    config.output->write(move(response));
}

LSPTask::Phase LSPRequestTask::finalPhase() const {
    return Phase::RUN;
}

bool LSPRequestTask::cancel(const MessageId &id) {
    if (this->id.equals(id)) {
        if (latencyTimer) {
            latencyTimer->cancel();
            latencyTimer = nullptr;
        }
        auto response = make_unique<ResponseMessage>("2.0", id, method);
        prodCategoryCounterInc("lsp.messages.canceled", methodString());
        response->error = make_unique<ResponseError>((int)LSPErrorCodes::RequestCancelled, "Request was canceled");
        config.output->write(move(response));
        return true;
    }
    return false;
}

bool LSPTask::needsMultithreading(const LSPIndexer &indexer) const {
    return false;
}

bool LSPTask::isDelayable() const {
    return false;
}

bool LSPTask::cancel(const MessageId &id) {
    return false;
}

bool LSPTask::canPreempt(const LSPIndexer &indexer) const {
    // A task that can preempt cannot be multithreaded.
    return !needsMultithreading(indexer);
}

bool LSPTask::canUseStaleData() const {
    return false;
}

vector<unique_ptr<Location>>
LSPTask::extractLocations(const core::GlobalState &gs,
                          const vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                          vector<unique_ptr<Location>> locations) const {
    auto queryResponsesFiltered = LSPQuery::filterAndDedup(gs, queryResponses);
    for (auto &q : queryResponsesFiltered) {
        if (auto *send = q->isSend()) {
            addLocIfExists(gs, locations, send->funLoc);
        } else {
            addLocIfExists(gs, locations, q->getLoc());
        }
    }
    return locations;
}

vector<unique_ptr<core::lsp::QueryResponse>>
LSPTask::getReferencesToSymbol(LSPTypecheckerInterface &typechecker, core::SymbolRef symbol,
                               vector<unique_ptr<core::lsp::QueryResponse>> &&priorRefs) const {
    if (symbol.exists()) {
        auto run2 = LSPQuery::bySymbol(config, typechecker, symbol);
        absl::c_move(run2.responses, back_inserter(priorRefs));
    }
    return move(priorRefs);
}

vector<unique_ptr<core::lsp::QueryResponse>>
LSPTask::getReferencesToSymbolInPackage(LSPTypecheckerInterface &typechecker, core::NameRef packageName,
                                        core::SymbolRef symbol,
                                        vector<unique_ptr<core::lsp::QueryResponse>> &&priorRefs) const {
    if (symbol.exists()) {
        auto run2 = LSPQuery::bySymbol(config, typechecker, symbol, packageName);
        absl::c_move(run2.responses, back_inserter(priorRefs));
    }
    return move(priorRefs);
}

vector<unique_ptr<core::lsp::QueryResponse>>
LSPTask::getReferencesToSymbolInFile(LSPTypecheckerInterface &typechecker, core::FileRef fref, core::SymbolRef symbol,
                                     vector<unique_ptr<core::lsp::QueryResponse>> &&priorRefs) const {
    if (symbol.exists() && fref.exists()) {
        auto run2 = LSPQuery::bySymbolInFiles(config, typechecker, symbol, {fref});
        for (auto &resp : run2.responses) {
            // Ignore results in other files (which may have been picked up for typechecking purposes)
            if (resp->getLoc().file() == fref) {
                priorRefs.emplace_back(move(resp));
            }
        }
    }
    return move(priorRefs);
}

vector<unique_ptr<DocumentHighlight>>
LSPTask::getHighlights(LSPTypecheckerInterface &typechecker,
                       const vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses) const {
    vector<unique_ptr<DocumentHighlight>> highlights;
    auto locations = extractLocations(typechecker.state(), queryResponses);
    for (auto const &location : locations) {
        auto highlight = make_unique<DocumentHighlight>(move(location->range));
        highlights.emplace_back(move(highlight));
    }
    return highlights;
}

namespace {

static const vector<core::NameRef> accessorNames = {
    core::Names::prop(),        core::Names::tokenProp(),    core::Names::timestampedTokenProp(),
    core::Names::createdProp(), core::Names::attrAccessor(),
};

static const vector<core::NameRef> writerNames = {
    core::Names::attrWriter(),
};

static const vector<core::NameRef> readerNames = {
    core::Names::const_(),
    core::Names::merchantProp(),
    core::Names::attrReader(),
};

void populateFieldAccessorType(const core::GlobalState &gs, AccessorInfo &info) {
    auto method = info.readerSymbol.exists() ? info.readerSymbol : info.writerSymbol;
    ENFORCE(method.exists());

    // Check definition site of method for `prop`, `const`, etc. The loc for the method should begin with
    // `def|prop|const|...`.
    auto methodSource = method.data(gs)->loc().source(gs);
    if (!methodSource.has_value()) {
        return;
    }
    // Common case: ordinary `def`. Fast reject.
    if (absl::StartsWith(methodSource.value(), "def")) {
        info.accessorType = FieldAccessorType::None;
        return;
    }

    if (absl::c_any_of(accessorNames, [&methodSource, &gs](auto name) -> bool {
            return absl::StartsWith(methodSource.value(), name.toString(gs));
        })) {
        info.accessorType = FieldAccessorType::Accessor;
    } else if (absl::c_any_of(writerNames, [&methodSource, &gs](auto name) -> bool {
                   return absl::StartsWith(methodSource.value(), name.toString(gs));
               })) {
        info.accessorType = FieldAccessorType::Writer;
    } else if (absl::c_any_of(readerNames, [&methodSource, &gs](auto name) -> bool {
                   return absl::StartsWith(methodSource.value(), name.toString(gs));
               })) {
        info.accessorType = FieldAccessorType::Reader;
    } else {
        info.accessorType = FieldAccessorType::None;
    }
}

} // namespace

vector<unique_ptr<core::lsp::QueryResponse>>
LSPTask::getReferencesToAccessor(LSPTypecheckerInterface &typechecker, const AccessorInfo info,
                                 core::SymbolRef fallback,
                                 vector<unique_ptr<core::lsp::QueryResponse>> &&priorRefs) const {
    switch (info.accessorType) {
        case FieldAccessorType::None:
            // Common case: Not an accessor.
            return getReferencesToSymbol(typechecker, fallback, move(priorRefs));
        case FieldAccessorType::Reader:
            return getReferencesToSymbol(typechecker, info.fieldSymbol,
                                         getReferencesToSymbol(typechecker, info.readerSymbol, move(priorRefs)));
        case FieldAccessorType::Writer:
            return getReferencesToSymbol(typechecker, info.fieldSymbol,
                                         getReferencesToSymbol(typechecker, info.writerSymbol, move(priorRefs)));
        case FieldAccessorType::Accessor:
            return getReferencesToSymbol(
                typechecker, info.fieldSymbol,
                getReferencesToSymbol(typechecker, info.writerSymbol,
                                      getReferencesToSymbol(typechecker, info.readerSymbol, move(priorRefs))));
    }
}

AccessorInfo LSPTask::getAccessorInfo(const core::GlobalState &gs, core::SymbolRef symbol) const {
    AccessorInfo info;

    core::SymbolRef owner = symbol.owner(gs);
    if (!owner.exists() || !owner.isClassOrModule()) {
        return info;
    }
    core::ClassOrModuleRef ownerCls = owner.asClassOrModuleRef();

    string_view baseName;

    string symbolName = symbol.name(gs).toString(gs);
    // Extract the base name from `symbol`.
    if (absl::StartsWith(symbolName, "@")) {
        if (!symbol.isField(gs)) {
            return info;
        }
        info.fieldSymbol = symbol.asFieldRef();
        baseName = string_view(symbolName).substr(1);
    } else if (absl::EndsWith(symbolName, "=")) {
        if (!symbol.isMethod()) {
            return info;
        }
        info.writerSymbol = symbol.asMethodRef();
        baseName = string_view(symbolName).substr(0, symbolName.length() - 1);
    } else {
        if (!symbol.isMethod()) {
            return info;
        }
        info.readerSymbol = symbol.asMethodRef();
        baseName = symbolName;
    }

    // Find the other associated symbols.
    if (!info.fieldSymbol.exists()) {
        auto fieldNameStr = absl::StrCat("@", baseName);
        auto fieldName = gs.lookupNameUTF8(fieldNameStr);
        if (!fieldName.exists()) {
            // Field is not optional.
            return info;
        }
        info.fieldSymbol = gs.lookupFieldSymbol(ownerCls, fieldName);
        if (!info.fieldSymbol.exists()) {
            // field symbol does not exist, so `symbol` must not be an accessor.
            return info;
        }
    }

    if (!info.readerSymbol.exists()) {
        auto readerName = gs.lookupNameUTF8(baseName);
        if (readerName.exists()) {
            info.readerSymbol = gs.lookupMethodSymbol(ownerCls, readerName);
        }
    }

    if (!info.writerSymbol.exists()) {
        auto writerNameStr = absl::StrCat(baseName, "=");
        auto writerName = gs.lookupNameUTF8(writerNameStr);
        if (writerName.exists()) {
            info.writerSymbol = gs.lookupMethodSymbol(ownerCls, writerName);
        }
    }

    // If this is an accessor, we should have a field and _at least_ one of reader or writer.
    if (!info.writerSymbol.exists() && !info.readerSymbol.exists()) {
        return info;
    }

    // Use reader or writer to determine what type of field accessor we are dealing with (if any).
    populateFieldAccessorType(gs, info);
    return info;
}

vector<unique_ptr<core::lsp::QueryResponse>>
LSPTask::getReferencesToAccessorInFile(LSPTypecheckerInterface &typechecker, core::FileRef fref,
                                       const AccessorInfo info, core::SymbolRef fallback,
                                       vector<unique_ptr<core::lsp::QueryResponse>> &&priorRefs) const {
    switch (info.accessorType) {
        case FieldAccessorType::None:
            // Common case: Not an accessor.
            return getReferencesToSymbolInFile(typechecker, fref, fallback, move(priorRefs));
        case FieldAccessorType::Reader:
            return getReferencesToSymbolInFile(
                typechecker, fref, info.fieldSymbol,
                getReferencesToSymbolInFile(typechecker, fref, info.readerSymbol, move(priorRefs)));
        case FieldAccessorType::Writer:
            return getReferencesToSymbolInFile(
                typechecker, fref, info.fieldSymbol,
                getReferencesToSymbolInFile(typechecker, fref, info.writerSymbol, move(priorRefs)));
        case FieldAccessorType::Accessor:
            return getReferencesToSymbolInFile(
                typechecker, fref, info.fieldSymbol,
                getReferencesToSymbolInFile(
                    typechecker, fref, info.writerSymbol,
                    getReferencesToSymbolInFile(typechecker, fref, info.readerSymbol, move(priorRefs))));
    }
}

void LSPTask::addLocIfExists(const core::GlobalState &gs, vector<unique_ptr<Location>> &locs, core::Loc loc) const {
    auto location = config.loc2Location(gs, loc);
    if (location != nullptr) {
        locs.push_back(std::move(location));
    }
}

LSPQueuePreemptionTask::LSPQueuePreemptionTask(const LSPConfiguration &config, absl::Notification &finished,
                                               TaskQueue &taskQueue, LSPIndexer &indexer)
    : LSPTask(config, LSPMethod::SorbetError), finished(finished), taskQueue(taskQueue), indexer(indexer) {}

void LSPQueuePreemptionTask::run(LSPTypecheckerInterface &tc) {
    for (;;) {
        unique_ptr<LSPTask> task;
        {
            absl::MutexLock lck(taskQueue.getMutex());
            if (taskQueue.tasks().empty() || !taskQueue.tasks().front()->canPreempt(indexer)) {
                break;
            }
            task = move(taskQueue.tasks().front());
            taskQueue.tasks().pop_front();

            {
                Timer timeit(config.logger, "LSPTask::index");
                timeit.setTag("method", task->methodString());
                // Index while holding lock to prevent races with processing thread.
                task->index(indexer);
            }
        }
        prodCategoryCounterInc("lsp.messages.processed", task->methodString());

        if (task->finalPhase() == Phase::INDEX) {
            continue;
        }
        Timer timeit(config.logger, "LSPTask::run");
        timeit.setTag("method", task->methodString());
        task->run(tc);
    }
    finished.Notify();
}

LSPDangerousTypecheckerTask::LSPDangerousTypecheckerTask(const LSPConfiguration &config, LSPMethod method)
    : LSPTask(config, method) {}

void LSPDangerousTypecheckerTask::run(LSPTypecheckerInterface &tc) {
    Exception::raise("Bug: Dangerous typechecker tasks are expected to run specially");
}

} // namespace sorbet::realmain::lsp
