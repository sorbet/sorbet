#ifndef SORBET_BUILDER_H
#define SORBET_BUILDER_H

#include "ast/ast.h"
#include "cfg/CFG.h"

namespace sorbet::cfg {
class CFGBuilder final {
public:
    static std::unique_ptr<CFG> buildFor(core::Context ctx, ast::MethodDef &md);

private:
    static BasicBlock *walk(CFGContext cctx, ast::ExpressionPtr &what, BasicBlock *current);
    static void fillInTopoSorts(core::Context ctx, CFG &cfg);
    static void dealias(core::Context ctx, CFG &cfg);
    static void simplify(core::Context ctx, CFG &cfg);
    static void sanityCheck(core::Context ctx, CFG &cfg);
    static std::vector<UIntSet> fillInBlockArguments(core::Context ctx, const CFG::ReadsAndWrites &RnW, const CFG &cfg);
    static void computeMinMaxLoops(core::Context ctx, const CFG::ReadsAndWrites &RnW, CFG &cfg);
    static void removeDeadAssigns(core::Context ctx, const CFG::ReadsAndWrites &RnW, CFG &cfg,
                                  const std::vector<UIntSet> &blockArgs);
    static void markLoopHeaders(core::Context ctx, CFG &cfg);
    static std::vector<int> topoSortFwd(std::vector<BasicBlock *> &target, int numBlocks, BasicBlock *currentBB);
    static void conditionalJump(BasicBlock *from, LocalRef cond, BasicBlock *thenb, BasicBlock *elseb, CFG &inWhat,
                                core::LocOffsets loc);
    static void unconditionalJump(BasicBlock *from, BasicBlock *to, CFG &inWhat, core::LocOffsets loc);
    static void jumpToDead(BasicBlock *from, CFG &inWhat, core::LocOffsets loc);
    static void synthesizeExpr(BasicBlock *bb, LocalRef var, core::LocOffsets loc, InstructionPtr inst);
    static BasicBlock *walkAssign(CFGContext cctx, ast::ExpressionPtr &rhs, core::LocOffsets assignLoc, LocalRef lhs,
                                  BasicBlock *current);
    static BasicBlock *walkHash(CFGContext cctx, ast::Hash &h, BasicBlock *current, core::NameRef method);
    static BasicBlock *walkEmptyTreeInIf(CFGContext cctx, core::LocOffsets loc, BasicBlock *current);
    static BasicBlock *walkBlockReturn(CFGContext cctx, core::LocOffsets loc, ast::ExpressionPtr &expr,
                                       BasicBlock *current);
    static std::tuple<LocalRef, BasicBlock *, BasicBlock *>
    walkDefault(CFGContext cctx, int paramIndex, const core::ParamInfo &paramInfo, LocalRef paramLocal,
                core::LocOffsets paramLoc, ast::ExpressionPtr &def, BasicBlock *presentCont, BasicBlock *defaultCont);
    static BasicBlock *joinBlocks(CFGContext cctx, BasicBlock *a, BasicBlock *b);
    static BasicBlock *buildExceptionHandler(CFGContext cctx, ast::ExpressionPtr &ex, BasicBlock *caseBody,
                                             cfg::LocalRef exceptionValue, BasicBlock *rescueHandlersBlock);
};

class CFGContext {
public:
    core::Context ctx;
    CFG &inWhat;
    LocalRef target;
    LocalRef blockBreakTarget;
    int loops;
    bool isInsideRubyBlock;
    bool isInsideLambda;
    bool breakIsJump;
    BasicBlock *nextScope;
    BasicBlock *breakScope;
    BasicBlock *rescueScope;
    std::shared_ptr<core::SendAndBlockLink> *link = nullptr;
    UnorderedMap<core::SymbolRef, LocalRef> &aliases;
    UnorderedMap<core::NameRef, LocalRef> &discoveredUndeclaredFields;

    uint32_t &temporaryCounter;

    CFGContext withTarget(LocalRef target);
    CFGContext withBlockBreakTarget(LocalRef blockBreakTarget);
    CFGContext withLoopBreakTarget(LocalRef blockBreakTarget);
    CFGContext withLoopScope(BasicBlock *nextScope, BasicBlock *breakScope, bool insideRubyBlock = false);
    CFGContext withSendAndBlockLink(std::shared_ptr<core::SendAndBlockLink> &link);

    LocalRef newTemporary(core::NameRef name);

private:
    friend std::unique_ptr<CFG> CFGBuilder::buildFor(core::Context ctx, ast::MethodDef &md);
    CFGContext(core::Context ctx, CFG &inWhat, LocalRef target, int loops, BasicBlock *nextScope,
               BasicBlock *breakScope, BasicBlock *rescueScope, UnorderedMap<core::SymbolRef, LocalRef> &aliases,
               UnorderedMap<core::NameRef, LocalRef> &discoveredUndeclaredFields, uint32_t &temporaryCounter)
        : ctx(ctx), inWhat(inWhat), target(target), loops(loops), isInsideRubyBlock(false), isInsideLambda(false),
          breakIsJump(false), nextScope(nextScope), breakScope(breakScope), rescueScope(rescueScope), aliases(aliases),
          discoveredUndeclaredFields(discoveredUndeclaredFields), temporaryCounter(temporaryCounter){};
};
} // namespace sorbet::cfg
#endif // SORBET_BUILDER_H
