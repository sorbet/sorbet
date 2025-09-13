#include "main/lsp/ExtractVariable.h"
#include "absl/strings/escaping.h"
#include "ast/treemap/treemap.h"
#include "common/sort/sort.h"

using namespace std;

namespace sorbet::realmain::lsp {

void logDebugInfo(const shared_ptr<spdlog::logger> logger, const core::GlobalState &gs, const core::Loc selectionLoc,
                  const string message) {
    logger->error("msg=\"ExtractToVariable: {}\" selectionLoc=\"{}\"", message, selectionLoc.showRaw(gs));
    logger->error("source=\"{}\"", absl::CEscape(selectionLoc.file().data(gs).source()));
}

optional<core::LocOffsets> detectCase(const ast::ExpressionPtr *whereToInsert, const core::LocOffsets target) {
    if (auto send = ast::cast_tree<ast::Send>(*whereToInsert)) {
        if (send->fun == core::Names::caseWhen()) {
            int numPatterns = core::cast_type_nonnull<core::IntegerLiteralType>(
                                  ast::cast_tree_nonnull<ast::Literal>(send->getPosArg(1)).value)
                                  .value;
            for (int i = numPatterns + 2; i < send->numPosArgs(); i++) {
                const auto &body = send->getPosArg(i);
                if (body.loc().exists() && body.loc().contains(target)) {
                    return body.loc();
                }
            }
        }
    }

    return nullopt;
}

core::LocOffsets findWhereToInsert(const ast::ExpressionPtr &scope, const core::LocOffsets target) {
    const ast::ExpressionPtr *whereToInsert = nullptr;
    if (auto insSeq = ast::cast_tree<ast::InsSeq>(scope)) {
        for (auto &stat : insSeq->stats) {
            if (stat.loc().contains(target)) {
                whereToInsert = &stat;
                break;
            }
        }
        if (insSeq->expr.loc().contains(target)) {
            whereToInsert = &insSeq->expr;
        }
    } else if (auto classDef = ast::cast_tree<ast::ClassDef>(scope)) {
        if (classDef->rhs.size() == 0) {
            ENFORCE(false);
        } else {
            for (auto &stat : classDef->rhs) {
                if (stat.loc().contains(target)) {
                    whereToInsert = &stat;
                    break;
                }
            }
        }
    } else if (auto block = ast::cast_tree<ast::Block>(scope)) {
        whereToInsert = &block->body;
    } else if (auto methodDef = ast::cast_tree<ast::MethodDef>(scope)) {
        whereToInsert = &methodDef->rhs;
    } else if (auto if_ = ast::cast_tree<ast::If>(scope)) {
        if (if_->thenp.loc().exists() && if_->thenp.loc().contains(target)) {
            whereToInsert = &if_->thenp;
        } else if (if_->elsep.loc().exists() && if_->elsep.loc().contains(target)) {
            whereToInsert = &if_->elsep;
        } else {
            ENFORCE(!(if_->cond.loc().exists() && if_->cond.loc().contains(target)),
                    "shouldn't be inserting into the cond of an if");
            ENFORCE(false, "none of the if sub-parts contain the target");
        }
    } else if (auto rescue = ast::cast_tree<ast::Rescue>(scope)) {
        if (rescue->body.loc().exists() && rescue->body.loc().contains(target)) {
            whereToInsert = &rescue->body;
        } else if (rescue->else_.loc().exists() && rescue->else_.loc().contains(target)) {
            whereToInsert = &rescue->else_;
        } else if (rescue->ensure.loc().exists() && rescue->ensure.loc().contains(target)) {
            whereToInsert = &rescue->ensure;
        } else {
            ENFORCE(false)
        }
    } else if (auto rescueCase = ast::cast_tree<ast::RescueCase>(scope)) {
        whereToInsert = &rescueCase->body;
    } else if (auto while_ = ast::cast_tree<ast::While>(scope)) {
        if (while_->body.loc().exists() && while_->body.loc().contains(target)) {
            whereToInsert = &while_->body;
        } else {
            ENFORCE(!(while_->cond.loc().exists() && while_->cond.loc().contains(target)),
                    "shouldn't be inserting into the cond of a while");
            ENFORCE(false, "none of the while sub-parts contain the target");
        }
    } else {
        ENFORCE(false);
    }

    ENFORCE(whereToInsert);
    if (!whereToInsert) {
        return core::LocOffsets::none();
    }

    if (auto caseBodyLoc = detectCase(whereToInsert, target)) {
        return *caseBodyLoc;
    }
    return whereToInsert->loc();
}

// This tree walk takes a Loc and looks for nodes that have that Loc exactly
class LocSearchWalk {
    // The selection loc
    core::Loc targetLoc;
    // At the end of this walk, we want to return what class/method the matching expression was part of.
    // To do that, we can maintain a stack of classes/method, so that when we get a match, we can capture
    // the current top of the stack as the "deepest" class/method
    vector<ast::ExpressionPtr *> enclosingClassStack;
    vector<ast::ExpressionPtr *> enclosingMethodStack;

    void updateEnclosingScope(const ast::ExpressionPtr &node, core::LocOffsets nodeLoc) {
        if (!nodeLoc.exists() || !nodeLoc.contains(targetLoc.offsets())) {
            return;
        }

        // If the new enclosingScope is an If node, and else branch is another if node, then this is probably an elsif
        // that was desugared to an if inside the else. In that case, insert into the else would be invalid, so let's
        // skip it. If there's a more specific scope, we'll insert there, and if there isn't, we'll insert outside the
        // if.
        if (auto if_ = ast::cast_tree<ast::If>(node)) {
            if (auto elsif = ast::cast_tree<ast::If>(if_->elsep)) {
                if (elsif->loc.exists() && elsif->loc.contains(targetLoc.offsets())) {
                    return;
                }
            }
        }

        if (!enclosingScopeLoc.exists() || enclosingScopeLoc.contains(nodeLoc)) {
            enclosingScope = &node;
            enclosingScopeLoc = nodeLoc;
            return;
        }
    }

    // Skip any node whose loc is contained by this loc
    void skipLocRange(core::LocOffsets loc) {
        if (loc.exists()) {
            skippedLocsRange.push_back(loc);
        }
    }
    //
    // Skip any loc whose loc exactly matches this loc
    void skipLocExact(core::LocOffsets loc) {
        if (loc.exists()) {
            skippedLocsExact.push_back(loc);
        }
    }

    // NOTE: Might want to profile and switch to UnorderedSet.
    bool shouldSkipLoc(core::LocOffsets loc) {
        return absl::c_find_if(skippedLocsRange, [loc](auto l) { return l.contains(loc); }) != skippedLocsRange.end() ||
               absl::c_find(skippedLocsExact, loc) != skippedLocsExact.end();
    }

public:
    // After the walk is complete, this should point to the deepest scope that contains targetLoc
    const ast::ExpressionPtr *enclosingScope;
    // enclosingScope is the ClassDef/MethodDef/etc. that contains targetLoc.
    // enclosingScopeLoc stores the Loc of the body of that scope.
    // For example, the RHS of the ClassDef doesn't have an ExpressionPtr with a Loc,
    // but enclosingScopeLoc will be a Loc that represents the body of the ClassDef RHS
    // (excluding things like the class name, superclass, and class/end keywords).
    // TODO(neil): can we get rid of enclosingScopeLoc?
    core::LocOffsets enclosingScopeLoc;
    const ast::ExpressionPtr *matchingNode;
    ast::ExpressionPtr *matchingNodeEnclosingClass;
    ast::ExpressionPtr *matchingNodeEnclosingMethod;
    // It's not valid to extract
    // - a parameter
    // - the lhs of an assign
    // - an endless method
    // - the var for a rescueCase
    // This vector stores the locs for those nodes, so that in preTransformExpression, we can skip them.
    vector<core::LocOffsets> skippedLocsRange;
    vector<core::LocOffsets> skippedLocsExact;

    LocSearchWalk(core::Loc targetLoc)
        : targetLoc(targetLoc), enclosingScopeLoc(core::LocOffsets::none()), matchingNode(nullptr),
          matchingNodeEnclosingClass(nullptr), matchingNodeEnclosingMethod(nullptr) {}

    void preTransformExpressionPtr(core::Context ctx, const ast::ExpressionPtr &tree) {
        if (tree.loc() == targetLoc.offsets()) {
            // It's not valid to extract the following node types
            if (ast::isa_tree<ast::Break>(tree) || ast::isa_tree<ast::Next>(tree) || ast::isa_tree<ast::Return>(tree) ||
                ast::isa_tree<ast::Retry>(tree) || ast::isa_tree<ast::RescueCase>(tree) ||
                ast::isa_tree<ast::InsSeq>(tree)) {
                return;
            }

            if (auto classDef = ast::cast_tree<ast::ClassDef>(tree)) {
                if (classDef->symbol == core::Symbols::root()) {
                    return;
                }
            }

            matchingNode = &tree;
            ENFORCE(!enclosingClassStack.empty(), "Must at least have the <root> ClassDef");
            matchingNodeEnclosingClass = enclosingClassStack.back();
            if (!enclosingMethodStack.empty()) {
                matchingNodeEnclosingMethod = enclosingMethodStack.back();
            }
        }
    }

    bool foundExactMatch() {
        return matchingNode && !shouldSkipLoc(matchingNode->loc());
    }

    void preTransformInsSeq(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(tree);
        updateEnclosingScope(tree, insSeq.loc);
    }

    void preTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        enclosingClassStack.push_back(&tree);
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);
        updateEnclosingScope(tree, classDef.rhs.front().loc().join(classDef.rhs.back().loc()));
    }

    void postTransformClassDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingClassStack.pop_back();
    }

    void preTransformMethodDef(core::Context ctx, ast::ExpressionPtr &tree) {
        enclosingMethodStack.push_back(&tree);
        auto &methodDef = ast::cast_tree_nonnull<ast::MethodDef>(tree);
        if (!methodDef.params.empty()) {
            skipLocRange(methodDef.params.front().loc().join(methodDef.params.back().loc()));
        }
        if (methodDef.loc.endPos() == methodDef.rhs.loc().endPos()) {
            // methodDef.loc.endPos() represent the location right after the `end`,
            // while methodDef.rhs.loc().endPos() is the location right before the `end`.
            // If both are the same, that means that this is an endless method.
            skipLocRange(methodDef.rhs.loc());
        } else {
            updateEnclosingScope(tree, methodDef.rhs.loc());
        }
    }

    void postTransformMethodDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingMethodStack.pop_back();
    }

    void preTransformBlock(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &block = ast::cast_tree_nonnull<ast::Block>(tree);
        if (!block.params.empty()) {
            skipLocRange(block.params.front().loc().join(block.params.back().loc()));
        }
        updateEnclosingScope(tree, block.body.loc());
    }

    void preTransformAssign(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &assign = ast::cast_tree_nonnull<ast::Assign>(tree);
        skipLocRange(assign.lhs.loc());
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
        skipLocRange(rescueCase.var.loc());
        for (auto &exception : rescueCase.exceptions) {
            skipLocRange(exception.loc());
        }
    }

    void preTransformWhile(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &while_ = ast::cast_tree_nonnull<ast::While>(tree);
        updateEnclosingScope(tree, while_.body.loc());
    }

    void preTransformSend(core::Context ctx, const ast::ExpressionPtr &tree) {
        auto &send = ast::cast_tree_nonnull<ast::Send>(tree);
        if (send.fun == core::Names::orAsgn() || send.fun == core::Names::andAsgn() ||
            send.fun == core::Names::opAsgn()) {
            ENFORCE(send.numPosArgs() >= 1);
            skipLocExact(send.getPosArg(0).loc());
        }

        if (auto constantLit = ast::cast_tree<ast::ConstantLit>(send.recv)) {
            if (constantLit->symbol() == core::Symbols::Regexp() && send.fun == core::Names::new_()) {
                ENFORCE(send.numPosArgs() >= 2);
                skipLocRange(send.getPosArg(0).loc());
                skipLocRange(send.getPosArg(1).loc());
            }
        }
    }
};

vector<unique_ptr<TextDocumentEdit>>
VariableExtractor::getExtractSingleOccurrenceEdits(const LSPTypecheckerDelegate &typechecker,
                                                   const LSPConfiguration &config) {
    const auto file = selectionLoc.file();
    const auto &gs = typechecker.state();

    LocSearchWalk walk(selectionLoc);
    auto afterLocalVars = typechecker.getLocalVarTrees(file);
    core::Context ctx(gs, core::Symbols::root(), file);
    ast::TreeWalk::apply(ctx, walk, afterLocalVars);

    if (!walk.foundExactMatch()) {
        return {};
    }

    auto locOffsets = selectionLoc.offsets();
    auto enclosingScope = walk.enclosingScope;
    auto whereToInsert = findWhereToInsert(*enclosingScope, locOffsets);
    if (!whereToInsert.exists()) {
        logDebugInfo(config.logger, gs, selectionLoc,
                     "failed to determine whereToInsert in getExtractSingleOccurrenceEdits");
        return {};
    }
    matchingNode = walk.matchingNode->deepCopy();
    if (walk.matchingNodeEnclosingMethod) {
        enclosingClassOrMethod = std::move(*walk.matchingNodeEnclosingMethod);
    } else {
        enclosingClassOrMethod = std::move(*walk.matchingNodeEnclosingClass);
    }
    skippedLocsRange = walk.skippedLocsRange;
    skippedLocsExact = walk.skippedLocsExact;

    auto whereToInsertLoc = core::Loc(file, whereToInsert.copyWithZeroLength());
    auto [startOfLine, numSpaces] = whereToInsertLoc.findStartOfIndentation(gs);

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

// This tree walk takes a ExpressionPtr and looks for nodes that are the same as that node
class ExpressionPtrSearchWalk {
    ast::ExpressionPtr *targetNode;
    vector<const ast::ExpressionPtr *> enclosingScopeStack;
    vector<core::LocOffsets> skippedLocsRange;
    vector<core::LocOffsets> skippedLocsExact;
    const shared_ptr<spdlog::logger> logger;
    const core::Loc selectionLoc;

    // NOTE: Might want to profile and switch to UnorderedSet.
    bool shouldSkipLoc(core::LocOffsets loc) {
        return absl::c_find_if(skippedLocsRange, [loc](auto l) { return l.contains(loc); }) != skippedLocsRange.end() ||
               absl::c_find(skippedLocsExact, loc) != skippedLocsExact.end();
    }

    void computeLCA(const core::LocOffsets matchLoc) {
        if (LCAScopeStack.empty()) {
            for (auto *scope : enclosingScopeStack) {
                const ast::ExpressionPtr *scopeToCompare = scope;
                const ast::ExpressionPtr *scopeToInsertIn = scope;
                if (ast::isa_tree<ast::If>(*scope)) {
                    auto &if_ = ast::cast_tree_nonnull<ast::If>(*scope);
                    if (if_.thenp.loc().exists() && if_.thenp.loc().contains(matchLoc)) {
                        scopeToCompare = &if_.thenp;
                    } else if (if_.elsep.loc().exists() && if_.elsep.loc().contains(matchLoc)) {
                        if (ast::isa_tree<ast::If>(if_.elsep)) {
                            // This is to handle elsif. See LocSearchWalk#updateEnclosingScope for a
                            // detailed explanation.
                            continue;
                        }
                        scopeToCompare = &if_.elsep;
                    } else if (if_.cond.loc().contains(matchLoc)) {
                        scopeToCompare = &if_.cond;
                        scopeToInsertIn = LCAScopeStack.back().second;
                    } else {
                        ENFORCE(false);
                    }
                } else if (ast::isa_tree<ast::Rescue>(*scope)) {
                    auto &rescue = ast::cast_tree_nonnull<ast::Rescue>(*scope);
                    if (rescue.body.loc().exists() && rescue.body.loc().contains(matchLoc)) {
                        scopeToCompare = &rescue.body;
                    } else if (rescue.else_.loc().exists() && rescue.else_.loc().contains(matchLoc)) {
                        scopeToCompare = &rescue.else_;
                    } else if (rescue.ensure.loc().exists() && rescue.ensure.loc().contains(matchLoc)) {
                        scopeToCompare = &rescue.ensure;
                    } else {
                        auto found = false;
                        for (auto &rescueCase : rescue.rescueCases) {
                            if (rescueCase.loc().exists() && rescueCase.loc().contains(matchLoc)) {
                                scopeToCompare = &rescueCase;
                                found = true;
                            }
                        }
                        ENFORCE(found, "didn't find match in any of the rescue cases");
                    }
                } else if (ast::isa_tree<ast::While>(*scope)) {
                    auto &while_ = ast::cast_tree_nonnull<ast::While>(*scope);
                    if (while_.body.loc().contains(matchLoc)) {
                        scopeToCompare = &while_.body;
                    } else if (while_.cond.loc().contains(matchLoc)) {
                        scopeToCompare = &while_.cond;
                        scopeToInsertIn = LCAScopeStack.back().second;
                    } else {
                        ENFORCE(false);
                    }
                }
                LCAScopeStack.push_back(make_pair(scopeToCompare->loc(), scopeToInsertIn));
            }
        } else {
            for (size_t i = 0; i < LCAScopeStack.size(); i++) {
                auto scopeToCompare = LCAScopeStack[i].first;
                if (!scopeToCompare.contains(matchLoc)) {
                    LCAScopeStack.erase(LCAScopeStack.begin() + i, LCAScopeStack.end());
                    break;
                }
            }
        }
    }

public:
    vector<pair<core::LocOffsets, const ast::ExpressionPtr *>> LCAScopeStack;
    vector<core::LocOffsets> matches;
    ExpressionPtrSearchWalk(ast::ExpressionPtr *matchingNode, vector<core::LocOffsets> skippedLocsRange,
                            vector<core::LocOffsets> skippedLocsExact, const shared_ptr<spdlog::logger> logger,
                            const core::Loc selectionLoc)
        : targetNode(matchingNode), skippedLocsRange(skippedLocsRange), skippedLocsExact(skippedLocsExact),
          logger(logger), selectionLoc(selectionLoc) {}

    void preTransformExpressionPtr(core::Context ctx, const ast::ExpressionPtr &tree) {
        if (!tree.loc().exists()) {
            return;
        }

        if (shouldSkipLoc(tree.loc())) {
            return;
        }

        if (absl::c_find(matches, tree.loc()) != matches.end()) {
            // We've already seen a node with the exact same loc before, so this is likely constructed by
            // desugar. Skip it.
            // If this ENFORCE occurs, we should update desugar to not do this when preserveConcreteSyntax is true
            ENFORCE(false, "found another node with a loc we've already seen");
            return;
        }

        if (targetNode->structurallyEqual(ctx, tree, selectionLoc.file())) {
            matches.emplace_back(tree.loc());
            computeLCA(tree.loc());
        }
    }

    void preTransformInsSeq(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.push_back(&tree);
    }

    void postTransformInsSeq(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.pop_back();
    }

    void preTransformClassDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.push_back(&tree);
    }

    void postTransformClassDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.pop_back();
    }

    void preTransformMethodDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.push_back(&tree);
    }

    void postTransformMethodDef(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.pop_back();
    }

    void preTransformBlock(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.push_back(&tree);
    }

    void postTransformBlock(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.pop_back();
    }

    void preTransformIf(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.push_back(&tree);
    }

    void postTransformIf(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.pop_back();
    }

    void preTransformRescue(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.push_back(&tree);
    }

    void postTransformRescue(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.pop_back();
    }

    void preTransformWhile(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.push_back(&tree);
    }

    void postTransformWhile(core::Context ctx, const ast::ExpressionPtr &tree) {
        enclosingScopeStack.pop_back();
    }
};

MultipleOccurrenceResult VariableExtractor::getExtractMultipleOccurrenceEdits(const LSPTypecheckerDelegate &typechecker,
                                                                              const LSPConfiguration &config) {
    ENFORCE(matchingNode, "getExtractMultipleOccurrenceEdits called before getExtractSingleOccurrenceEdits");
    ENFORCE(enclosingClassOrMethod, "getExtractMultipleOccurrenceEdits called before getExtractSingleOccurrenceEdits");

    const auto file = selectionLoc.file();
    const auto &gs = typechecker.state();

    ExpressionPtrSearchWalk walk(&matchingNode, skippedLocsRange, skippedLocsExact, config.logger, selectionLoc);
    core::Context ctx(gs, core::Symbols::root(), file);
    ast::TreeWalk::apply(ctx, walk, enclosingClassOrMethod);

    auto matches = walk.matches;

    // There should be at least one match (the original selected expression).
    ENFORCE(!matches.empty());
    if (matches.size() == 1) {
        return {vector<unique_ptr<TextDocumentEdit>>(), 1};
    }

    const ast::ExpressionPtr *scopeToInsertIn = walk.LCAScopeStack.back().second;

    fast_sort(matches, [](auto a, auto b) { return a.beginPos() < b.beginPos(); });
    auto firstMatch = matches[0];

    auto whereToInsert = findWhereToInsert(*scopeToInsertIn, firstMatch);
    if (!whereToInsert.exists()) {
        logDebugInfo(config.logger, gs, selectionLoc,
                     "failed to determine whereToInsert in getExtractMultipleOccurrenceEdits");
        return {};
    }
    auto whereToInsertLoc = core::Loc(file, whereToInsert.copyWithZeroLength());
    auto [startOfLine, numSpaces] = whereToInsertLoc.findStartOfIndentation(gs);

    auto trailing = whereToInsertLoc.beginPos() == startOfLine.beginPos()
                        // If we're inserting at the start of the line (ignoring whitespace),
                        // let's put the declaration on a new line (above the current one) instead.
                        ? fmt::format("\n{}", string(numSpaces, ' '))
                        : "; ";

    vector<unique_ptr<TextEdit>> edits;
    if (whereToInsertLoc.endPos() == firstMatch.beginPos()) {
        // if insertion point is touching the first match, let's merge the 2 edits into one,
        // to prevent an "overlapping" edit.
        whereToInsertLoc = whereToInsertLoc.join(core::Loc(file, firstMatch));
        edits.emplace_back(make_unique<TextEdit>(
            Range::fromLoc(gs, whereToInsertLoc),
            fmt::format("newVariable = {}{}newVariable", selectionLoc.source(gs).value(), trailing)));
        for (int j = 1; j < matches.size(); j++) {
            auto match = matches[j];
            auto matchLoc = Range::fromLoc(gs, core::Loc(file, match));
            edits.emplace_back(make_unique<TextEdit>(std::move(matchLoc), "newVariable"));
        }
    } else {
        edits.emplace_back(
            make_unique<TextEdit>(Range::fromLoc(gs, whereToInsertLoc),
                                  fmt::format("newVariable = {}{}", selectionLoc.source(gs).value(), trailing)));
        for (auto match : matches) {
            auto matchLoc = Range::fromLoc(gs, core::Loc(file, match));
            edits.emplace_back(make_unique<TextEdit>(std::move(matchLoc), "newVariable"));
        }
    }

    auto docEdit = make_unique<TextDocumentEdit>(
        make_unique<VersionedTextDocumentIdentifier>(config.fileRef2Uri(gs, file), JSONNullObject()), move(edits));

    vector<unique_ptr<TextDocumentEdit>> res;
    res.emplace_back(move(docEdit));
    return {move(res), matches.size()};
}
} // namespace sorbet::realmain::lsp
