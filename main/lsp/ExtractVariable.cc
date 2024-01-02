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

    void updateEnclosingScope(ast::ExpressionPtr &node, core::LocOffsets nodeLoc) {
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
        return absl::c_find(skippedLocs, loc) != skippedLocs.end();
    }

public:
    // After the walk is complete, this should point to the deepest scope
    // (block, method or class) that contains targetLoc
    ast::ExpressionPtr *enclosingScope;
    core::LocOffsets enclosingScopeLoc;
    // Is there a syntax node that has the exact same location as targetLoc?

    ExtractVariableWalk(core::Loc targetLoc)
        : targetLoc(targetLoc), matchingLoc(core::LocOffsets::none()), enclosingScopeLoc(core::LocOffsets::none()) {}

    void preTransformExpression(core::Context ctx, ast::ExpressionPtr &tree) {
        if (tree.loc() == targetLoc.offsets()) {
            // It's not valid to extract the following node types
            if (!ast::isa_tree<ast::Break>(tree) && !ast::isa_tree<ast::Next>(tree) &&
                !ast::isa_tree<ast::Return>(tree) && !ast::isa_tree<ast::Retry>(tree) &&
                !ast::isa_tree<ast::RescueCase>(tree)) {
                matchingLoc = tree.loc();
            }
        }
    }

    bool foundExactMatch() {
        return matchingLoc.exists() && !shouldSkipLoc(matchingLoc);
    }

    void preTransformInsSeq(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(tree);
        updateEnclosingScope(tree, insSeq.loc);
    }

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        updateEnclosingScope(tree, classDef.rhs.front().loc().join(classDef.rhs.back().loc()));
    }

    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        for (auto &arg : methodDef.args) {
            skipLoc(arg.loc());
        }
        updateEnclosingScope(tree, methodDef.rhs.loc());
    }

    void preTransformBlock(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &block = ast::cast_tree_nonnull<ast::Block>(tree);
        for (auto &arg : block.args) {
            skipLoc(arg.loc());
        }
        updateEnclosingScope(tree, block.body.loc());
    }

    void preTransformAssign(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &assign = ast::cast_tree_nonnull<ast::Assign>(tree);
        skipLoc(assign.lhs.loc());
    }

    void preTransformIf(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &if_ = ast::cast_tree_nonnull<ast::If>(tree);
        updateEnclosingScope(tree, if_.thenp.loc());
        updateEnclosingScope(tree, if_.elsep.loc());
    }

    void preTransformRescue(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &rescue = ast::cast_tree_nonnull<ast::Rescue>(tree);
        updateEnclosingScope(tree, rescue.body.loc());
        updateEnclosingScope(tree, rescue.else_.loc());
        updateEnclosingScope(tree, rescue.ensure.loc());
    }

    void preTransformRescueCase(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &rescueCase = ast::cast_tree_nonnull<ast::RescueCase>(tree);
        updateEnclosingScope(tree, rescueCase.body.loc());
        skipLoc(rescueCase.var.loc());
        for (auto &exception : rescueCase.exceptions) {
            skipLoc(exception.loc());
        }
    }

    void preTransformWhile(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &while_ = ast::cast_tree_nonnull<ast::While>(tree);
        updateEnclosingScope(tree, while_.body.loc());
    }
};

vector<unique_ptr<TextDocumentEdit>> getExtractVariableEdits(LSPTypecheckerDelegate &typechecker,
                                                             const LSPConfiguration &config,
                                                             unique_ptr<Range> selectionRange,
                                                             const core::Loc selectionLoc) {
    auto loc = selectionLoc;
    auto file = loc.file();
    const auto &gs = typechecker.state();
    vector<unique_ptr<TextEdit>> edits;

    ExtractVariableWalk extractVariableWalk(loc);
    auto desugaredTree = typechecker.getDesugared(file);
    core::Context ctx(gs, core::Symbols::root(), file);
    ast::TreeWalk::apply(ctx, extractVariableWalk, desugaredTree);

    if (extractVariableWalk.foundExactMatch()) {
        vector<unique_ptr<TextEdit>> edits;

        auto locOffsets = loc.offsets();
        auto whereToInsert = core::LocOffsets::none();
        auto enclosingScope = extractVariableWalk.enclosingScope;
        // For all cases except InsSeq and ClassDef, extractVariableWalk.enclosingScopeLoc should be
        // the same as what we're pulling out from the ExpressionPtr, but we'll just get it directly
        // for consistency.
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
            ENFORCE(whereToInsert.exists());
        } else if (auto block = ast::cast_tree<ast::Block>(*enclosingScope)) {
            ENFORCE(!ast::isa_tree<ast::InsSeq>(block->body));
            whereToInsert = block->body.loc();
        } else if (auto methodDef = ast::cast_tree<ast::MethodDef>(*enclosingScope)) {
            // TODO(neil): this will fail for endless methods
            ENFORCE(!ast::isa_tree<ast::InsSeq>(methodDef->rhs));
            whereToInsert = methodDef->rhs.loc();
        } else if (auto classDef = ast::cast_tree<ast::ClassDef>(*enclosingScope)) {
            // TODO(neil): this would be much simpler if we just had ClassDef
            // do the same thing as methodDef and block (ClassDef#rhs is a ExpressionPtr
            // and we use InsSeq for multiple statements). Maybe we can change ClassDef to do that?
            if (classDef->rhs.size() == 0) {
                ENFORCE(false);
            } else if (classDef->rhs.size() == 1) {
                ENFORCE(!ast::isa_tree<ast::InsSeq>(classDef->rhs.front()));
                whereToInsert = classDef->rhs.front().loc();
            } else {
                for (auto &stat : classDef->rhs) {
                    if (stat.loc().contains(locOffsets)) {
                        ENFORCE(!ast::isa_tree<ast::InsSeq>(stat));
                        whereToInsert = stat.loc();
                        break;
                    }
                }
                ENFORCE(whereToInsert.exists());
            }
        } else if (auto if_ = ast::cast_tree<ast::If>(*enclosingScope)) {
            if (if_->thenp.loc().contains(locOffsets)) {
                ENFORCE(!ast::isa_tree<ast::InsSeq>(if_->thenp));
                whereToInsert = if_->thenp.loc();
            } else {
                ENFORCE(!ast::isa_tree<ast::InsSeq>(if_->elsep));
                whereToInsert = if_->elsep.loc();
            }
        } else if (auto rescue = ast::cast_tree<ast::Rescue>(*enclosingScope)) {
            if (rescue->body.loc().contains(locOffsets)) {
                ENFORCE(!ast::isa_tree<ast::InsSeq>(rescue->body));
                whereToInsert = rescue->body.loc();
            } else if (rescue->else_.loc().contains(locOffsets)) {
                ENFORCE(!ast::isa_tree<ast::InsSeq>(rescue->else_));
                whereToInsert = rescue->else_.loc();
            } else if (rescue->ensure.loc().contains(locOffsets)) {
                ENFORCE(!ast::isa_tree<ast::InsSeq>(rescue->ensure));
                whereToInsert = rescue->ensure.loc();
            } else {
                ENFORCE(false)
            }
        } else if (auto rescueCase = ast::cast_tree<ast::RescueCase>(*enclosingScope)) {
            ENFORCE(!ast::isa_tree<ast::InsSeq>(rescueCase->body));
            whereToInsert = rescueCase->body.loc();
        } else if (auto while_ = ast::cast_tree<ast::While>(*enclosingScope)) {
            ENFORCE(!ast::isa_tree<ast::InsSeq>(while_->body));
            whereToInsert = while_->body.loc();
        } else {
            ENFORCE(false);
        }
        auto whereToInsertLoc = core::Loc(file, whereToInsert.copyWithZeroLength());
        auto whereToInsertRange = Range::fromLoc(gs, whereToInsertLoc);
        auto [startOfLine, numSpaces] = whereToInsertLoc.findStartOfLine(gs);

        string trailing;
        if (whereToInsertLoc.beginPos() == startOfLine.beginPos()) {
            // If we're inserting at the start of the line (ignoring whitespace),
            // let's put the declaration on a new line (above the current one) instead.
            trailing = fmt::format("\n{}", string(numSpaces, ' '));
        } else {
            trailing = fmt::format("; ", string(numSpaces, ' '));
        }
        if (whereToInsertLoc.endPos() == loc.beginPos()) {
            // if insertion point is touching the selection, let's merge the 2 edits into one,
            // to prevent an "overlapping" edit.
            whereToInsertLoc = whereToInsertLoc.join(loc);
            whereToInsertRange = Range::fromLoc(gs, whereToInsertLoc);
            edits.emplace_back(
                make_unique<TextEdit>(move(whereToInsertRange),
                                      fmt::format("newVariable = {}{}newVariable", loc.source(gs).value(), trailing)));
        } else {
            edits.emplace_back(make_unique<TextEdit>(
                move(whereToInsertRange), fmt::format("newVariable = {}{}", loc.source(gs).value(), trailing)));
            edits.emplace_back(make_unique<TextEdit>(std::move(selectionRange), "newVariable"));
        }
        auto docEdit = make_unique<TextDocumentEdit>(
            make_unique<VersionedTextDocumentIdentifier>(config.fileRef2Uri(gs, file), JSONNullObject()), move(edits));

        vector<unique_ptr<TextDocumentEdit>> res;
        res.emplace_back(move(docEdit));
        return res;
    }

    return {};
}
} // namespace sorbet::realmain::lsp
