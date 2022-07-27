#include "main/lsp/requests/document_symbol.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

#include "absl/algorithm/container.h"

using namespace std;

namespace sorbet::realmain::lsp {
std::unique_ptr<DocumentSymbol> symbolRef2DocumentSymbol(const core::GlobalState &gs, core::SymbolRef symRef,
                                                         core::FileRef filter);

void symbolRef2DocumentSymbolWalkMembers(const core::GlobalState &gs, core::SymbolRef sym, core::FileRef filter,
                                         vector<unique_ptr<DocumentSymbol>> &out) {
    if (!sym.isClassOrModule()) {
        return;
    }

    for (auto mem : sym.asClassOrModuleRef().data(gs)->membersStableOrderSlow(gs)) {
        if (mem.first == core::Names::attached() || mem.first == core::Names::singleton()) {
            continue;
        }
        if (!absl::c_any_of(mem.second.locs(gs), [&filter](const auto &loc) { return loc.file() == filter; })) {
            continue;
        }
        auto inner = symbolRef2DocumentSymbol(gs, mem.second, filter);
        if (inner) {
            out.push_back(move(inner));
        }
    }
}

std::unique_ptr<DocumentSymbol> symbolRef2DocumentSymbol(const core::GlobalState &gs, core::SymbolRef symRef,
                                                         core::FileRef filter) {
    if (!symRef.exists()) {
        return nullptr;
    }
    auto loc = symRef.loc(gs);
    if (!loc.file().exists() || hideSymbol(gs, symRef)) {
        return nullptr;
    }
    auto kind = symbolRef2SymbolKind(gs, symRef);
    // TODO: this range should cover body. Currently it doesn't.
    auto range = Range::fromLoc(gs, loc);
    auto selectionRange = Range::fromLoc(gs, loc);
    if (range == nullptr || selectionRange == nullptr) {
        return nullptr;
    }

    string prefix;
    auto owner = symRef.owner(gs);
    if (owner.exists() && owner.isClassOrModule() && owner.asClassOrModuleRef().data(gs)->attachedClass(gs).exists()) {
        prefix = "self.";
    }
    auto result =
        make_unique<DocumentSymbol>(prefix + symRef.name(gs).show(gs), kind, move(range), move(selectionRange));

    // Previous versions of VSCode have a bug that requires this non-optional field to be present.
    // This previously tried to include the method signature but due to issues where large signatures were not readable
    // when put on one line and given that currently details are only visible in the outline view but not seen in the
    // symbol search. Additionally, no other language server implementations we could find used this field.
    result->detail = "";

    vector<unique_ptr<DocumentSymbol>> children;
    symbolRef2DocumentSymbolWalkMembers(gs, symRef, filter, children);
    if (symRef.isClassOrModule()) {
        auto singleton = symRef.asClassOrModuleRef().data(gs)->lookupSingletonClass(gs);
        if (singleton.exists()) {
            symbolRef2DocumentSymbolWalkMembers(gs, singleton, filter, children);
        }
    }
    result->children = move(children);
    return result;
}

DocumentSymbolTask::DocumentSymbolTask(const LSPConfiguration &config, MessageId id,
                                       std::unique_ptr<DocumentSymbolParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentDocumentSymbol), params(move(params)) {}

bool DocumentSymbolTask::isDelayable() const {
    return true;
}

unique_ptr<ResponseMessage> DocumentSymbolTask::runRequest(LSPTypecheckerInterface &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentDocumentSymbol);
    if (!config.opts.lspDocumentSymbolEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Document Symbol` LSP feature is experimental and disabled by default.");
        return response;
    }

    const core::GlobalState &gs = typechecker.state();
    vector<unique_ptr<DocumentSymbol>> result;
    string_view uri = params->textDocument->uri;
    auto fref = config.uri2FileRef(gs, uri);
    vector<pair<core::SymbolRef::Kind, uint32_t>> symbolTypes = {
        {core::SymbolRef::Kind::ClassOrModule, gs.classAndModulesUsed()},
        {core::SymbolRef::Kind::Method, gs.methodsUsed()},
        {core::SymbolRef::Kind::FieldOrStaticField, gs.fieldsUsed()},
        {core::SymbolRef::Kind::TypeArgument, gs.typeArgumentsUsed()},
        {core::SymbolRef::Kind::TypeMember, gs.typeMembersUsed()},
    };
    vector<core::SymbolRef> candidates;
    for (auto [kind, used] : symbolTypes) {
        for (uint32_t idx = 1; idx < used; idx++) {
            core::SymbolRef ref(gs, kind, idx);
            if (hideSymbol(gs, ref)) {
                continue;
            }

            // If the owner lives in this file, then the owner will be caught elsewhere
            // in this loop and `ref` will be output as a child of owner.  So we should
            // avoid outputting `ref` at this point.  Symbols owned by root, however,
            // should always be output, as we don't output root itself.
            if (ref.owner(gs).loc(gs).file() == fref && ref.owner(gs) != core::Symbols::root()) {
                continue;
            }

            for (auto definitionLocation : ref.locs(gs)) {
                if (definitionLocation.file() != fref) {
                    continue;
                }

                candidates.emplace_back(ref);
            }
        }
    }

    // Despite our best efforts above, we might still output duplicates from the
    // list of candidate symbols.  For instance, given an ownership chain:
    //
    // A -> B -> C
    //
    // Consider the case where B was defined in a different file and C was defined
    // in the current file: then C will be in our candidate list.
    //
    // But if B has a loc in the current file and A was defined in a different file,
    // then B will also be in our candidate list!
    //
    // To avoid that case, we need to walk the ownership chains of each symbol to
    // deduplicate the candidate list.
    UnorderedSet<core::SymbolRef> deduplicatedCandidates(candidates.begin(), candidates.end());
    for (auto ref : candidates) {
        auto owner = ref.owner(gs);
        while (owner != core::Symbols::root()) {
            if (deduplicatedCandidates.contains(owner)) {
                deduplicatedCandidates.erase(ref);
                break;
            }

            owner = owner.owner(gs);
        }
    }
    candidates.erase(
        std::remove_if(candidates.begin(), candidates.end(),
                       [&deduplicatedCandidates](const auto ref) { return !deduplicatedCandidates.contains(ref); }),
        candidates.end());

    for (auto ref : candidates) {
        auto data = symbolRef2DocumentSymbol(gs, ref, fref);
        if (data) {
            result.push_back(move(data));
        }
    }
    response->result = move(result);
    return response;
}

} // namespace sorbet::realmain::lsp
