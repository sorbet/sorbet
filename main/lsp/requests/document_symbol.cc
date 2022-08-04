#include "main/lsp/requests/document_symbol.h"
#include "ast/treemap/treemap.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

#include "absl/algorithm/container.h"

using namespace std;

namespace sorbet::realmain::lsp {
namespace {
std::unique_ptr<DocumentSymbol> symbolRef2DocumentSymbol(const core::GlobalState &gs, core::SymbolRef symRef,
                                                         core::FileRef filter,
                                                         const UnorderedMap<core::SymbolRef, core::Loc> &defMapping,
                                                         const UnorderedSet<core::SymbolRef> &forced);

void symbolRef2DocumentSymbolWalkMembers(const core::GlobalState &gs, core::SymbolRef sym, core::FileRef filter,
                                         const UnorderedMap<core::SymbolRef, core::Loc> &defMapping,
                                         const UnorderedSet<core::SymbolRef> &forced,
                                         vector<unique_ptr<DocumentSymbol>> &out) {
    if (!sym.isClassOrModule()) {
        return;
    }

    for (auto mem : sym.asClassOrModuleRef().data(gs)->membersStableOrderSlow(gs)) {
        if (mem.first == core::Names::attached() || mem.first == core::Names::singleton()) {
            continue;
        }
        if (!absl::c_any_of(mem.second.locs(gs), [&filter](const auto &loc) { return loc.file() == filter; })) {
            if (!forced.contains(mem.second)) {
                continue;
            }
        }
        auto inner = symbolRef2DocumentSymbol(gs, mem.second, filter, defMapping, forced);
        if (inner) {
            out.push_back(move(inner));
        }
    }
}

std::unique_ptr<DocumentSymbol> symbolRef2DocumentSymbol(const core::GlobalState &gs, core::SymbolRef symRef,
                                                         core::FileRef filter,
                                                         const UnorderedMap<core::SymbolRef, core::Loc> &defMapping,
                                                         const UnorderedSet<core::SymbolRef> &forced) {
    if (!symRef.exists()) {
        return nullptr;
    }
    auto loc = symRef.loc(gs);
    if (!loc.file().exists() || hideSymbol(gs, symRef)) {
        return nullptr;
    }
    auto kind = symbolRef2SymbolKind(gs, symRef);
    auto it = defMapping.find(symRef);
    unique_ptr<Range> range;
    if (it != defMapping.end()) {
        range = Range::fromLoc(gs, it->second);
    }
    if (range == nullptr) {
        range = Range::fromLoc(gs, loc);
    }
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
    symbolRef2DocumentSymbolWalkMembers(gs, symRef, filter, defMapping, forced, children);
    if (symRef.isClassOrModule()) {
        auto singleton = symRef.asClassOrModuleRef().data(gs)->lookupSingletonClass(gs);
        if (singleton.exists()) {
            symbolRef2DocumentSymbolWalkMembers(gs, singleton, filter, defMapping, forced, children);
        }
    }
    result->children = move(children);
    return result;
}

// Document symbols in the LSP spec include two different ranges: one that covers
// the entire extent of the definition and one that covers just what should be
// shown when the symbol is selected.  Sorbet's loc for a symbol covers the latter.
// We do store the former, but that loc lives in the AST, not in the symbol table,
// so we build a separate temporary mapping with this tree walker.
class DefinitionLocSaver {
public:
    core::FileRef fref;
    UnorderedMap<core::SymbolRef, core::Loc> &mapping;

    DefinitionLocSaver(core::FileRef fref, UnorderedMap<core::SymbolRef, core::Loc> &mapping)
        : fref(fref), mapping(mapping) {}

    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &expr) {
        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(expr);
        if (!klass.symbol.exists()) {
            return;
        }

        mapping[klass.symbol] = core::Loc{fref, klass.loc};
    }

    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &expr) {
        auto &method = ast::cast_tree_nonnull<ast::MethodDef>(expr);
        if (!method.symbol.exists()) {
            return;
        }

        mapping[method.symbol] = core::Loc{fref, method.loc};
    }
};

} // namespace

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
    if (!fref.exists()) {
        response->result = std::move(result);
        return response;
    }

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

            if (absl::c_any_of(ref.locs(gs), [&fref](const auto &loc) { return loc.file() == fref; })) {
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

    // We may wind up in a situation where we have something like:
    //
    // A::B::C::D::E
    //
    // B and E are candidates, but neither C nor D have locs in the current
    // file.  We'll eliminate E from candidacy, but then when we go to get
    // children for B, we find that C doesn't appear in the current file, so
    // we won't walk C's children.  Which means we'll never walk D's children
    // and therefore E won't appear in the list of document symbols.
    //
    // So we need to ensure that both C and D are displayed; this set records
    // symbols that appear in an ownership chain between two candidates.
    UnorderedSet<core::SymbolRef> forced;
    for (auto ref : candidates) {
        auto owner = ref.owner(gs);
        while (owner != core::Symbols::root()) {
            if (deduplicatedCandidates.contains(owner)) {
                deduplicatedCandidates.erase(ref);
                auto forcedOwner = ref.owner(gs);
                while (forcedOwner != owner) {
                    forced.insert(forcedOwner);
                    forcedOwner = forcedOwner.owner(gs);
                }
                break;
            }

            owner = owner.owner(gs);
        }
    }
    candidates.erase(
        std::remove_if(candidates.begin(), candidates.end(),
                       [&deduplicatedCandidates](const auto ref) { return !deduplicatedCandidates.contains(ref); }),
        candidates.end());

    UnorderedMap<core::SymbolRef, core::Loc> defMapping;
    DefinitionLocSaver saver{fref, defMapping};
    core::Context ctx{gs, core::Symbols::root(), fref};
    auto resolved = typechecker.getResolved({fref});
    ast::TreeWalk::apply(ctx, saver, resolved[0].tree);

    for (auto ref : candidates) {
        auto data = symbolRef2DocumentSymbol(gs, ref, fref, defMapping, forced);
        if (data) {
            result.push_back(move(data));
        }
    }
    response->result = move(result);
    return response;
}

} // namespace sorbet::realmain::lsp
