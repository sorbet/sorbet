#include "main/lsp/requests/document_symbol.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

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
        if (mem.first != core::Names::attached() && mem.first != core::Names::singleton()) {
            bool foundThisFile = false;
            for (auto loc : mem.second.locs(gs)) {
                foundThisFile = foundThisFile || loc.file() == filter;
            }
            if (!foundThisFile) {
                continue;
            }
            auto inner = symbolRef2DocumentSymbol(gs, mem.second, filter);
            if (inner) {
                out.push_back(move(inner));
            }
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
    for (auto [kind, used] : symbolTypes) {
        for (uint32_t idx = 1; idx < used; idx++) {
            core::SymbolRef ref(gs, kind, idx);
            if (!hideSymbol(gs, ref) &&
                // a bit counter-intuitive, but this actually should be `!= fref`, as it prevents duplicates.
                (ref.owner(gs).loc(gs).file() != fref || ref.owner(gs) == core::Symbols::root())) {
                for (auto definitionLocation : ref.locs(gs)) {
                    if (definitionLocation.file() == fref) {
                        auto data = symbolRef2DocumentSymbol(gs, ref, fref);
                        if (data) {
                            result.push_back(move(data));
                            break;
                        }
                    }
                }
            }
        }
    }
    response->result = move(result);
    return response;
}

} // namespace sorbet::realmain::lsp
