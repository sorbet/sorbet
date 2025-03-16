#include "main/lsp/ExtractMethod.h"
#include "absl/strings/str_join.h"
#include "ast/treemap/treemap.h"

using namespace std;

namespace sorbet::realmain::lsp {

class Walk {
    // The selection loc
    core::LocOffsets targetLoc;

public:
    // After the walk is complete, this should point to the deepest insSeq that contains targetLoc
    const ast::InsSeq *enclosingInsSeq;
    const ast::MethodDef *enclosingMethodDef;
    Walk(core::LocOffsets targetLoc) : targetLoc(targetLoc), enclosingInsSeq(nullptr) {}

    void preTransformInsSeq(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(tree);
        if (insSeq.loc.exists() && insSeq.loc.contains(targetLoc)) {
            if (!enclosingInsSeq || enclosingInsSeq->loc.contains(insSeq.loc)) {
                enclosingInsSeq = &insSeq;
            }
        }
    }

    void preTransformMethodDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        if (methodDef.loc.exists() && methodDef.loc.contains(targetLoc)) {
            enclosingMethodDef = &methodDef;
        }
    }
};

class FindLocalsWalk {
    absl::flat_hash_set<core::NameRef> &localsAssigned;
    absl::flat_hash_set<core::NameRef> &localsReferenced;

    absl::flat_hash_set<core::LocOffsets> lhsLocs;

public:
    FindLocalsWalk(absl::flat_hash_set<core::NameRef> &localsAssigned,
                   absl::flat_hash_set<core::NameRef> &localsReferenced)
        : localsAssigned(localsAssigned), localsReferenced(localsReferenced) {}
    void preTransformAssign(core::Context ctx, const ast::Assign &assign) {
        if (auto local = ast::cast_tree<ast::Local>(assign.lhs)) {
            localsAssigned.insert(local->localVariable._name);
            lhsLocs.insert(local->loc);
        }
        return;
    }

    void postTransformLocal(core::Context ctx, const ast::Local &local) {
        if (!lhsLocs.contains(local.loc)) {
            localsReferenced.insert(local.localVariable._name);
        }
    }
};

void findLocals(const core::GlobalState &gs, const core::FileRef file,
                absl::flat_hash_set<core::NameRef> &localsAssigned,
                absl::flat_hash_set<core::NameRef> &localsReferenced, const ast::ExpressionPtr *insn) {
    FindLocalsWalk walk(localsAssigned, localsReferenced);
    core::Context ctx(gs, core::Symbols::root(), file);
    ast::ConstTreeWalk::apply(ctx, walk, *insn);
}

const ast::ExpressionPtr *getElem(const ast::InsSeq *insSeq, int i) {
    if (i > insSeq->stats.size()) {
        return nullptr;
    }

    if (i == insSeq->stats.size()) {
        return &insSeq->expr;
    }

    return &insSeq->stats[i];
}

vector<unique_ptr<TextDocumentEdit>> MethodExtractor::getExtractEdits(const LSPTypecheckerDelegate &typechecker,
                                                                      const LSPConfiguration &config) {
    const auto file = selectionLoc.file();
    const auto &gs = typechecker.state();
    // Steps:
    // - Find enclosing InsSeq (insSeq.contains(selectionLoc))
    Walk walk(selectionLoc.offsets());
    // TODO(neil): do we need local Vars?
    auto afterLocalVars = typechecker.getLocalVarTrees(file);
    core::Context ctx(gs, core::Symbols::root(), file);
    ast::TreeWalk::apply(ctx, walk, afterLocalVars);
    auto *enclosingInsSeq = walk.enclosingInsSeq;
    auto *enclosingMethodDef = walk.enclosingMethodDef;

    if (!enclosingInsSeq || !enclosingMethodDef) {
        return {};
    }
    //
    // - Step through each insn in InsSeq, and see if selection corrosponds to full insns
    //   - start of selection == start of some insn and end of selection == end of some insn
    int start_idx = -1;
    int end_idx = -1;
    // TODO: use vector instead of set so it's ordered? or btree_set?
    absl::flat_hash_set<core::NameRef> localsAssignedBefore;
    // TODO: replace with std::transform/absl::c_transform?
    for (auto &param : enclosingMethodDef->args) {
        if (auto local = ast::cast_tree<ast::Local>(param)) {
            localsAssignedBefore.insert(local->localVariable._name);
        }
    }
    absl::flat_hash_set<core::NameRef> localsReferencedIn;
    absl::flat_hash_set<core::NameRef> localsAssignedIn;
    absl::flat_hash_set<core::NameRef> localsReferencedAfter;
    // size() + 1 to account for InsSeq#expr
    // TODO: we need to find all the locals from before the enclosing insSeq too
    for (int i = 0; i < enclosingInsSeq->stats.size() + 1; i++) {
        auto *insn = getElem(enclosingInsSeq, i);
        if (start_idx == -1) {
            // TODO: move the start of selectionLoc to the first non-whitespace character, so we can ignore the
            // indentation at the start
            if (insn->loc().beginPos() == selectionLoc.offsets().beginPos()) {
                // First insn in the user's selection
                start_idx = i;

                findLocals(gs, file, localsAssignedIn, localsReferencedIn, insn);
            } else {
                // Before the user's selection
                absl::flat_hash_set<core::NameRef> localsReferencedBefore_;
                findLocals(gs, file, localsAssignedBefore, localsReferencedBefore_, insn);
            }
        } else if (end_idx == -1) {
            // Inside the user's selection
            findLocals(gs, file, localsAssignedIn, localsReferencedIn, insn);

            // TODO: move the end of selectionLoc to the last non-whitespace character, so we can ignore the
            // indentation at the end
            if (insn->loc().endPos() == selectionLoc.offsets().endPos()) {
                // Last insn in the user's selection
                end_idx = i;
            }
        } else {
            // After the user's selection
            absl::flat_hash_set<core::NameRef> localsAssignedAfter_;
            findLocals(gs, file, localsAssignedAfter_, localsReferencedAfter, insn);
        }
    }

    if (start_idx == -1 || end_idx == -1) {
        return {};
    }
    /* for (auto name : localsAssignedBefore) { */
    /*     fmt::print("assignedBefore: {}\n", name.toString(gs)); */
    /* } */
    /* for (auto name : localsAssignedIn) { */
    /*     fmt::print("assignedIn: {}\n", name.toString(gs)); */
    /* } */
    /* for (auto name : localsReferencedIn) { */
    /*     fmt::print("referencedIn: {}\n", name.toString(gs)); */
    /* } */
    /* for (auto name : localsReferencedAfter) { */
    /*     fmt::print("referencedAfter: {}\n", name.toString(gs)); */
    /* } */
    // TODO: if the selection includes a return, don't allow extracting it for now?

    std::vector<string> methodParams;
    std::vector<string> methodReturns;
    // args for new method are those that are (referencedIn and (assignedBefore or !assignedIn))
    for (auto name : localsReferencedIn) {
        if (name == core::Names::selfLocal()) {
            continue;
        }
        if (localsAssignedBefore.contains(name) || !localsAssignedIn.contains(name)) {
            methodParams.push_back(name.show(gs));
        }
    }
    // new method will return those are (assignedIn and referencedAfter)
    for (auto name : localsAssignedIn) {
        /* if (localsReferencedAfter.contains(name)) { */
        methodReturns.push_back(name.show(gs));
        /* } */
    }

    // TODO: indentation is wrong
    string firstLine = fmt::format("def newMethod({})", absl::StrJoin(methodParams, ", "));
    string lastLine = fmt::format("return {}", absl::StrJoin(methodReturns, ", "));
    auto newMethod = fmt::format("{}\n  {}\n  {}\nend\n", firstLine, selectionLoc.source(gs).value(), lastLine);

    vector<unique_ptr<TextEdit>> edits;
    edits.emplace_back(make_unique<TextEdit>(
        Range::fromLoc(gs, core::Loc(file, enclosingMethodDef->loc.copyWithZeroLength())), newMethod));
    if (methodReturns.size() > 0) {
        edits.emplace_back(make_unique<TextEdit>(
            Range::fromLoc(gs, selectionLoc),
            fmt::format("{} = newMethod({})", absl::StrJoin(methodReturns, ", "), absl::StrJoin(methodParams, ", "))));
    } else {
        edits.emplace_back(make_unique<TextEdit>(Range::fromLoc(gs, selectionLoc),
                                                 fmt::format("newMethod({})", absl::StrJoin(methodParams, ", "))));
    }

    auto docEdit = make_unique<TextDocumentEdit>(
        make_unique<VersionedTextDocumentIdentifier>(config.fileRef2Uri(gs, file), JSONNullObject()), move(edits));

    vector<unique_ptr<TextDocumentEdit>> res;
    res.emplace_back(move(docEdit));
    return res;
}

} // namespace sorbet::realmain::lsp
