#include "main/lsp/ExtractVariable.h"
#include "ast/treemap/treemap.h"

using namespace std;

namespace sorbet::realmain::lsp {

class ExtractVariableWalk {
    // The selection loc
    core::Loc targetLoc;
    core::LocOffsets matchingLoc;
    // It's not valid to extract a parameter, or the lhs of an assign.
    // This vector stores the locs for those nodes, so that in
    // preTransformExpression, we can skip them.
    std::vector<core::LocOffsets> skippedLocs;

    void updateEnclosingScope(const ast::ExpressionPtr &node, core::LocOffsets nodeLoc) {
        if (!nodeLoc.exists() || !nodeLoc.contains(targetLoc.offsets())) {
            return;
        }

        if (!enclosingScopeLoc.exists() || enclosingScopeLoc.contains(nodeLoc)) {
            enclosingScope = &node;
            enclosingScopeLoc = nodeLoc;
            return;
        }
    }

    void skipLoc(core::LocOffsets loc) {
        if (loc.exists()) {
            skippedLocs.push_back(loc);
        }
    }

    // NOTE: Might want to profile and switch to UnorderedSet.
    bool shouldSkipLoc(core::LocOffsets loc) {
        return absl::c_find_if(skippedLocs, [loc](auto l) { return l.contains(loc); }) != skippedLocs.end();
    }

public:
    // After the walk is complete, this should point to the deepest scope that contains targetLoc
    const ast::ExpressionPtr *enclosingScope;
    // enclosingScope is the ClassDef/MethodDef/etc. that contains targetLoc.
    // enclosingScopeLoc stores the Loc of the body of that scope.
    // For example, the RHS of the ClassDef doesn't have an ExpressionPtr with a Loc,
    // but enclosingScopeLoc will be a Loc that represents the body of the ClassDef RHS
    // (excluding things like the class name, superclass, and class/end keywords).
    core::LocOffsets enclosingScopeLoc;

    ExtractVariableWalk(core::Loc targetLoc)
        : targetLoc(targetLoc), matchingLoc(core::LocOffsets::none()), enclosingScopeLoc(core::LocOffsets::none()) {}

    void preTransformExpressionPtr(core::Context ctx, const ast::ExpressionPtr &tree) {
        if (tree.loc() == targetLoc.offsets()) {
            // It's not valid to extract the following node types
            if (!ast::isa_tree<ast::Break>(tree) && !ast::isa_tree<ast::Next>(tree) &&
                !ast::isa_tree<ast::Return>(tree) && !ast::isa_tree<ast::Retry>(tree) &&
                !ast::isa_tree<ast::RescueCase>(tree) && !ast::isa_tree<ast::InsSeq>(tree)) {
                matchingLoc = tree.loc();
            }
        }
    }

    bool foundExactMatch() {
        return matchingLoc.exists() && !shouldSkipLoc(matchingLoc);
    }

    void preTransformInsSeq(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(tree);
        updateEnclosingScope(tree, insSeq.loc);
    }

    void preTransformClassDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        updateEnclosingScope(tree, classDef.rhs.front().loc().join(classDef.rhs.back().loc()));
    }

    void preTransformMethodDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        if (!methodDef.args.empty()) {
            skipLoc(methodDef.args.front().loc().join(methodDef.args.back().loc()));
        }
        if (methodDef.loc.endPos() == methodDef.rhs.loc().endPos()) {
            // methodDef.loc.endPos() represent the location right after the `end`,
            // while methodDef.rhs.loc().endPos() is the location right before the `end`.
            // If both are the same, that means that this is an endless method.
            skipLoc(methodDef.rhs.loc());
        } else {
            updateEnclosingScope(tree, methodDef.rhs.loc());
        }
    }

    void preTransformBlock(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &block = ast::cast_tree_nonnull<ast::Block>(tree);
        if (!block.args.empty()) {
            skipLoc(block.args.front().loc().join(block.args.back().loc()));
        }
        updateEnclosingScope(tree, block.body.loc());
    }

    void preTransformAssign(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &assign = ast::cast_tree_nonnull<ast::Assign>(tree);
        skipLoc(assign.lhs.loc());
    }

    void preTransformIf(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &if_ = ast::cast_tree_nonnull<ast::If>(tree);
        updateEnclosingScope(tree, if_.thenp.loc());
        updateEnclosingScope(tree, if_.elsep.loc());
    }

    void preTransformRescue(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &rescue = ast::cast_tree_nonnull<ast::Rescue>(tree);
        updateEnclosingScope(tree, rescue.body.loc());
        updateEnclosingScope(tree, rescue.else_.loc());
        updateEnclosingScope(tree, rescue.ensure.loc());
    }

    void preTransformRescueCase(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &rescueCase = ast::cast_tree_nonnull<ast::RescueCase>(tree);
        updateEnclosingScope(tree, rescueCase.body.loc());
        skipLoc(rescueCase.var.loc());
        for (auto &exception : rescueCase.exceptions) {
            skipLoc(exception.loc());
        }
    }

    void preTransformWhile(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &while_ = ast::cast_tree_nonnull<ast::While>(tree);
        updateEnclosingScope(tree, while_.body.loc());
    }
};

vector<unique_ptr<TextDocumentEdit>> VariableExtractor::getEdits(LSPTypecheckerDelegate &typechecker,
                                                                 const LSPConfiguration &config,
                                                                 const core::Loc selectionLoc) {
    auto file = selectionLoc.file();
    const auto &gs = typechecker.state();

    ExtractVariableWalk extractVariableWalk(selectionLoc);
    auto desugaredTree = typechecker.getDesugared(file);
    core::Context ctx(gs, core::Symbols::root(), file);
    ast::TreeWalk::apply(ctx, extractVariableWalk, desugaredTree);

    if (!extractVariableWalk.foundExactMatch()) {
        return {};
    }

    auto locOffsets = selectionLoc.offsets();
    auto whereToInsert = core::LocOffsets::none();
    auto enclosingScope = extractVariableWalk.enclosingScope;
    // For all cases except InsSeq and ClassDef, extractVariableWalk.enclosingScopeLoc should be
    // the same as what we're pulling out from the ExpressionPtr, but we'll just get it directly
    // for consistency.
    // The ENFORCE(!ast::isa_tree<ast::InsSeq>(...)) are there check that the enclosingScope returned
    // by the TreeWalk doesn't contain a further InsSeq, because the preTransformInsSeq should have
    // matched on that (if it contains the selectionLoc).
    if (auto insSeq = ast::cast_tree<ast::InsSeq>(*enclosingScope)) {
        for (auto &stat : insSeq->stats) {
            if (stat.loc().contains(locOffsets)) {
                ENFORCE(!ast::isa_tree<ast::InsSeq>(stat));
                whereToInsert = stat.loc();
                break;
            }
        }
        if (insSeq->expr.loc().contains(locOffsets)) {
            ENFORCE(!ast::isa_tree<ast::InsSeq>(insSeq->expr));
            whereToInsert = insSeq->expr.loc();
        }
    } else if (auto classDef = ast::cast_tree<ast::ClassDef>(*enclosingScope)) {
        if (classDef->rhs.size() == 0) {
            ENFORCE(false);
        } else {
            for (auto &stat : classDef->rhs) {
                if (stat.loc().contains(locOffsets)) {
                    ENFORCE(!ast::isa_tree<ast::InsSeq>(stat));
                    whereToInsert = stat.loc();
                    break;
                }
            }
        }
    } else if (auto block = ast::cast_tree<ast::Block>(*enclosingScope)) {
        ENFORCE(!ast::isa_tree<ast::InsSeq>(block->body));
        ENFORCE(block->body.loc() == extractVariableWalk.enclosingScopeLoc,
                "loc on node doesn't match the loc found in the tree walk");
        whereToInsert = block->body.loc();
    } else if (auto methodDef = ast::cast_tree<ast::MethodDef>(*enclosingScope)) {
        // TODO(neil): this will fail for endless methods
        ENFORCE(!ast::isa_tree<ast::InsSeq>(methodDef->rhs));
        ENFORCE(methodDef->rhs.loc() == extractVariableWalk.enclosingScopeLoc,
                "loc on node doesn't match the loc found in the tree walk");
        whereToInsert = methodDef->rhs.loc();
    } else if (auto if_ = ast::cast_tree<ast::If>(*enclosingScope)) {
        if (if_->thenp.loc().exists() && if_->thenp.loc().contains(locOffsets)) {
            ENFORCE(!ast::isa_tree<ast::InsSeq>(if_->thenp));
            whereToInsert = if_->thenp.loc();
            ENFORCE(if_->thenp.loc() == extractVariableWalk.enclosingScopeLoc,
                    "loc on node doesn't match the loc found in the tree walk");
        } else {
            ENFORCE(!ast::isa_tree<ast::InsSeq>(if_->elsep));
            whereToInsert = if_->elsep.loc();
            ENFORCE(if_->elsep.loc() == extractVariableWalk.enclosingScopeLoc,
                    "loc on node doesn't match the loc found in the tree walk");
        }
    } else if (auto rescue = ast::cast_tree<ast::Rescue>(*enclosingScope)) {
        if (rescue->body.loc().exists() && rescue->body.loc().contains(locOffsets)) {
            ENFORCE(!ast::isa_tree<ast::InsSeq>(rescue->body));
            ENFORCE(rescue->body.loc() == extractVariableWalk.enclosingScopeLoc,
                    "loc on node doesn't match the loc found in the tree walk");
            whereToInsert = rescue->body.loc();
        } else if (rescue->else_.loc().exists() && rescue->else_.loc().contains(locOffsets)) {
            ENFORCE(!ast::isa_tree<ast::InsSeq>(rescue->else_));
            ENFORCE(rescue->else_.loc() == extractVariableWalk.enclosingScopeLoc,
                    "loc on node doesn't match the loc found in the tree walk");
            whereToInsert = rescue->else_.loc();
        } else if (rescue->ensure.loc().exists() && rescue->ensure.loc().contains(locOffsets)) {
            ENFORCE(!ast::isa_tree<ast::InsSeq>(rescue->ensure));
            ENFORCE(rescue->ensure.loc() == extractVariableWalk.enclosingScopeLoc,
                    "loc on node doesn't match the loc found in the tree walk");
            whereToInsert = rescue->ensure.loc();
        } else {
            ENFORCE(false)
        }
    } else if (auto rescueCase = ast::cast_tree<ast::RescueCase>(*enclosingScope)) {
        ENFORCE(!ast::isa_tree<ast::InsSeq>(rescueCase->body));
        ENFORCE(rescueCase->body.loc() == extractVariableWalk.enclosingScopeLoc,
                "loc on node doesn't match the loc found in the tree walk");
        whereToInsert = rescueCase->body.loc();
    } else if (auto while_ = ast::cast_tree<ast::While>(*enclosingScope)) {
        ENFORCE(!ast::isa_tree<ast::InsSeq>(while_->body));
        ENFORCE(while_->body.loc() == extractVariableWalk.enclosingScopeLoc,
                "loc on node doesn't match the loc found in the tree walk");
        whereToInsert = while_->body.loc();
    } else {
        ENFORCE(false);
    }
    ENFORCE(whereToInsert.exists());

    auto whereToInsertLoc = core::Loc(file, whereToInsert.copyWithZeroLength());
    auto [startOfLine, numSpaces] = whereToInsertLoc.findStartOfLine(gs);

    auto trailing = whereToInsertLoc.beginPos() == startOfLine.beginPos()
                        // If we're inserting at the start of the line (ignoring whitespace),
                        // let's put the declaration on a new line (above the current one) instead.
                        ? fmt::format("\n{}", string(numSpaces, ' '))
                        : "; ";

    vector<unique_ptr<TextEdit>> edits;

    if (whereToInsertLoc.endPos() == selectionLoc.beginPos()) {
        // if insertion point is touching the selection, let's merge the 2 edits into one,
        // to prevent an "overlapping" edit.
        whereToInsertLoc = whereToInsertLoc.join(selectionLoc);
        edits.emplace_back(make_unique<TextEdit>(
            Range::fromLoc(gs, whereToInsertLoc),
            fmt::format("newVariable = {}{}newVariable", selectionLoc.source(gs).value(), trailing)));
    } else {
        edits.emplace_back(
            make_unique<TextEdit>(Range::fromLoc(gs, whereToInsertLoc),
                                  fmt::format("newVariable = {}{}", selectionLoc.source(gs).value(), trailing)));
        auto selectionRange = Range::fromLoc(gs, selectionLoc);
        edits.emplace_back(make_unique<TextEdit>(std::move(selectionRange), "newVariable"));
    }
    auto docEdit = make_unique<TextDocumentEdit>(
        make_unique<VersionedTextDocumentIdentifier>(config.fileRef2Uri(gs, file), JSONNullObject()), move(edits));

    vector<unique_ptr<TextDocumentEdit>> res;
    res.emplace_back(move(docEdit));
    return res;
}
} // namespace sorbet::realmain::lsp
