#ifndef SORBET_BUILDER_H
#define SORBET_BUILDER_H

#include "ast/ast.h"
#include "cfg/CFG.h"

namespace sorbet::cfg {
class CFGBuilder final {
public:
    static std::unique_ptr<CFG> buildFor(core::Context ctx, ast::MethodDef &md);

private:
    static BasicBlock *walk(CFGContext cctx, ast::Expression *what, BasicBlock *current);
    static void fillInTopoSorts(core::Context ctx, CFG &cfg);
    static void dealias(core::Context ctx, CFG &cfg);
    static void simplify(core::Context ctx, CFG &cfg);
    static void sanityCheck(core::Context ctx, CFG &cfg);
    static void fillInBlockArguments(core::Context ctx, CFG::ReadsAndWrites &RnW, CFG &cfg);
    static void computeMinMaxLoops(core::Context ctx, const CFG::ReadsAndWrites &RnW, CFG &cfg);
    static void removeDeadAssigns(core::Context ctx, const CFG::ReadsAndWrites &RnW, CFG &cfg);
    static void markLoopHeaders(core::Context ctx, CFG &cfg);
    static int topoSortFwd(std::vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB);
};

class CFGContext {
public:
    core::Context ctx;
    CFG &inWhat;
    core::LocalVariable target;
    int loops;
    core::SymbolRef rubyBlock;
    BasicBlock *nextScope;
    BasicBlock *breakScope;
    BasicBlock *rescueScope;
    std::shared_ptr<core::SendAndBlockLink> link;
    UnorderedMap<core::SymbolRef, core::LocalVariable> &aliases;
    UnorderedMap<core::NameRef, core::LocalVariable> &discoveredUndeclaredFields;

    u4 &temporaryCounter;

    CFGContext withTarget(core::LocalVariable target);
    CFGContext withLoopScope(BasicBlock *nextScope, BasicBlock *breakScope,
                             core::SymbolRef rubyBlock = core::Symbols::noSymbol());
    CFGContext withSendAndBlockLink(const std::shared_ptr<core::SendAndBlockLink> &link);

    core::LocalVariable newTemporary(core::NameRef name);

private:
    friend std::unique_ptr<CFG> CFGBuilder::buildFor(core::Context ctx, ast::MethodDef &md);
    CFGContext(core::Context ctx, CFG &inWhat, core::LocalVariable target, int loops, BasicBlock *nextScope,
               BasicBlock *breakScope, BasicBlock *rescueScope,
               UnorderedMap<core::SymbolRef, core::LocalVariable> &aliases,
               UnorderedMap<core::NameRef, core::LocalVariable> &discoveredUndeclaredFields, u4 &temporaryCounter)
        : ctx(ctx), inWhat(inWhat), target(target), loops(loops), nextScope(nextScope), breakScope(breakScope),
          rescueScope(rescueScope), aliases(aliases), discoveredUndeclaredFields(discoveredUndeclaredFields),
          temporaryCounter(temporaryCounter){};
};
} // namespace sorbet::cfg
#endif // SORBET_BUILDER_H
