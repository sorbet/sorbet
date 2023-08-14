#include "main/lsp/requests/document_symbol.h"
#include "absl/strings/match.h"
#include "ast/treemap/treemap.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/json_types.h"

#include "absl/algorithm/container.h"

using namespace std;

namespace sorbet::realmain::lsp {
namespace {

// Document symbols in the LSP spec include two different ranges: one that covers
// the entire extent of the definition and one that covers just what should be
// shown when the symbol is selected.  Sorbet's loc for a symbol covers the latter
// (e.g. ClassDef::declLoc, MethodDef::declLoc).
//
// We do store the former, but that loc lives in the AST, not in the symbol table
// (e.g. ClassDef::loc, MethodDef::loc).  So we have to walk the AST to retrieve
// that loc...and then we might as well record `declLoc` when we do so we can
// ensure that both of the locs we're looking at are referencing the same file.
// (We could do this with the locs from the symbol table, but it seems simpler
// to do things this way.)
struct ASTSymbolInfo {
    core::Loc loc;
    core::Loc declLoc;
    bool isAttrBestEffortUIOnly;
};

class SymbolFileLocSaver {
public:
    core::FileRef fref;
    UnorderedMap<core::SymbolRef, ASTSymbolInfo> &mapping;

    SymbolFileLocSaver(core::FileRef fref, UnorderedMap<core::SymbolRef, ASTSymbolInfo> &mapping)
        : fref(fref), mapping(mapping) {}

    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &expr) {
        auto &klass = ast::cast_tree_nonnull<ast::ClassDef>(expr);
        if (!klass.symbol.exists()) {
            return;
        }

        auto isAttrBestEffortUIOnly = false;
        mapping[klass.symbol] =
            ASTSymbolInfo{core::Loc{fref, klass.loc}, core::Loc{fref, klass.declLoc}, isAttrBestEffortUIOnly};
    }

    void postTransformMethodDef(core::Context ctx, ast::ExpressionPtr &expr) {
        auto &method = ast::cast_tree_nonnull<ast::MethodDef>(expr);
        if (!method.symbol.exists()) {
            return;
        }

        auto isAttrBestEffortUIOnly = method.flags.isAttrBestEffortUIOnly;
        mapping[method.symbol] =
            ASTSymbolInfo{core::Loc{fref, method.loc}, core::Loc{fref, method.declLoc}, isAttrBestEffortUIOnly};
    }
};

std::unique_ptr<DocumentSymbol> symbolRef2DocumentSymbol(const core::GlobalState &gs, core::SymbolRef symRef,
                                                         core::FileRef filter,
                                                         const UnorderedMap<core::SymbolRef, ASTSymbolInfo> &defMapping,
                                                         const UnorderedMap<core::SymbolRef, core::SymbolRef> &forced);

void symbolRef2DocumentSymbolWalkMembers(const core::GlobalState &gs, core::SymbolRef sym, core::FileRef filter,
                                         const UnorderedMap<core::SymbolRef, ASTSymbolInfo> &defMapping,
                                         const UnorderedMap<core::SymbolRef, core::SymbolRef> &forced,
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

struct RangeInfo {
    unique_ptr<Range> range;
    unique_ptr<Range> selectionRange;
};

std::optional<RangeInfo> rangesForSymbol(const core::GlobalState &gs, core::SymbolRef symRef,
                                         const UnorderedMap<core::SymbolRef, ASTSymbolInfo> &defMapping,
                                         const UnorderedMap<core::SymbolRef, core::SymbolRef> &forced) {
    RangeInfo info;
    auto it = defMapping.find(symRef);
    if (it != defMapping.end()) {
        info.range = Range::fromLoc(gs, it->second.loc);
        info.selectionRange = Range::fromLoc(gs, it->second.declLoc);
    } else {
        auto it = forced.find(symRef);
        if (it != forced.end()) {
            return rangesForSymbol(gs, it->second, defMapping, forced);
        }

        auto loc = symRef.loc(gs);
        if (!loc.file().exists()) {
            return nullopt;
        }

        info.range = Range::fromLoc(gs, loc);
        info.selectionRange = Range::fromLoc(gs, loc);
    }

    if (info.range == nullptr || info.selectionRange == nullptr) {
        return nullopt;
    }

    return info;
}

bool symbolIsAttr(core::SymbolRef symRef, const UnorderedMap<core::SymbolRef, ASTSymbolInfo> &defMapping) {
    auto it = defMapping.find(symRef);
    return it != defMapping.end() && it->second.isAttrBestEffortUIOnly;
}

std::unique_ptr<DocumentSymbol> symbolRef2DocumentSymbol(const core::GlobalState &gs, core::SymbolRef symRef,
                                                         core::FileRef filter,
                                                         const UnorderedMap<core::SymbolRef, ASTSymbolInfo> &defMapping,
                                                         const UnorderedMap<core::SymbolRef, core::SymbolRef> &forced) {
    if (!symRef.exists()) {
        return nullptr;
    }
    if (hideSymbol(gs, symRef)) {
        return nullptr;
    }
    auto info = rangesForSymbol(gs, symRef, defMapping, forced);
    if (!info.has_value()) {
        return nullptr;
    }
    auto kind = symbolRef2SymbolKind(gs, symRef, symbolIsAttr(symRef, defMapping));

    string name;
    auto owner = symRef.owner(gs);
    if (owner.exists() && owner.isClassOrModule() && owner.asClassOrModuleRef().data(gs)->attachedClass(gs).exists()) {
        name = "self.";
    }
    auto symName = symRef.name(gs).show(gs);
    string_view view{symName};
    if (absl::StartsWith(view, "<") && view.size() > 1) {
        string_view describeStr = "<describe '";
        string_view itStr = "<it '";
        if (absl::StartsWith(view, describeStr) || absl::StartsWith(view, itStr)) {
            view.remove_prefix(1);
            view.remove_suffix(1);
        }
    }
    name += view;
    auto result = make_unique<DocumentSymbol>(move(name), kind, move(info->range), move(info->selectionRange));

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

} // namespace

DocumentSymbolTask::DocumentSymbolTask(const LSPConfiguration &config, MessageId id,
                                       std::unique_ptr<DocumentSymbolParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentDocumentSymbol), params(move(params)) {}

bool DocumentSymbolTask::isDelayable() const {
    return true;
}

unique_ptr<ResponseMessage> DocumentSymbolTask::runRequest(LSPTypecheckerDelegate &typechecker) {
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
    // module A::B; class C::D::E; ...; end; end
    //
    // B and E are candidates, but neither C nor D have locs in the current
    // file.  We'll eliminate E from candidacy, but then when we go to get
    // children for B, we find that C doesn't appear in the current file, so
    // we won't walk C's children.  Which means we'll never walk D's children
    // and therefore E won't appear in the list of document symbols.
    //
    // So we need to ensure that both C and D are represented in the document
    // symbols for the file.  But this raises a separate issue: what ranges do
    // we assign to the symbols for C and D?  Giving them ranges representing
    // their actual definitions is not correct, because their locs appear in an
    // entirely different file.  Giving them null ranges seems reasonable, but
    // that solution breaks cursor following in VSCode.
    //
    // As a sort of hacky compromise, we will give them ranges corresponding to
    // the nearest owner that appears in the file.  For C and D above, that
    // would be B.  (It would probably be more technically correct to give C and
    // D ranges corresponding to E, but this is complicated to do with our setup
    // here.).
    //
    // This map's keys record symbols that appear in an ownership chain between two
    // candidates; the corresponding values are the closest ancestor that appears
    // in this file.
    UnorderedMap<core::SymbolRef, core::SymbolRef> forced;
    for (auto ref : candidates) {
        auto owner = ref.owner(gs);
        while (owner != core::Symbols::root()) {
            if (deduplicatedCandidates.contains(owner)) {
                deduplicatedCandidates.erase(ref);
                auto forcedOwner = ref.owner(gs);
                while (forcedOwner != owner) {
                    // We could record `owner` here, but `owner` itself may
                    // eventually wind up in `forced`, which implies that the
                    // value for `forcedOwner` should really be something
                    // further up the ownership chain.  We'll fixup the values
                    // later once we're sure of where all the original candidates
                    // go.
                    forced[forcedOwner] = core::Symbols::noSymbol();
                    forcedOwner = forcedOwner.owner(gs);
                }
                break;
            }

            owner = owner.owner(gs);
        }
    }

    for (auto &slot : forced) {
        ENFORCE(!deduplicatedCandidates.contains(slot.first));
        auto owner = slot.first.owner(gs);
        while (!deduplicatedCandidates.contains(owner)) {
            ENFORCE(owner != core::Symbols::root());
            ENFORCE(owner.exists());
            owner = owner.owner(gs);
        }
        ENFORCE(owner.exists());
        slot.second = owner;
    }

    candidates.erase(
        std::remove_if(candidates.begin(), candidates.end(),
                       [&deduplicatedCandidates](const auto ref) { return !deduplicatedCandidates.contains(ref); }),
        candidates.end());

    UnorderedMap<core::SymbolRef, ASTSymbolInfo> defMapping;
    SymbolFileLocSaver saver{fref, defMapping};
    core::Context ctx{gs, core::Symbols::root(), fref};
    auto resolved = typechecker.getResolved({fref});
    for (auto &f : resolved) {
        ast::ShallowWalk::apply(ctx, saver, f.tree);
    }

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
