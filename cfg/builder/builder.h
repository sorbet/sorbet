#ifndef SRUBY_BUILDER_H
#define SRUBY_BUILDER_H

#include "ast/ast.h"
#include "cfg/CFG.h"

namespace ruby_typer {
namespace cfg {
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
    static int topoSortBwd(std::vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB);
};

class CFGContext {
public:
    core::Context ctx;
    CFG &inWhat;
    core::LocalVariable target;
    int loops;
    BasicBlock *scope;
    BasicBlock *rescueScope;
    std::unordered_map<core::SymbolRef, core::LocalVariable> &aliases;

    CFGContext withTarget(core::LocalVariable target);
    CFGContext withScope(BasicBlock *scope);

private:
    friend std::unique_ptr<CFG> CFGBuilder::buildFor(core::Context ctx, ast::MethodDef &md);
    CFGContext(core::Context ctx, CFG &inWhat, core::LocalVariable target, int loops, BasicBlock *scope,
               BasicBlock *rescueScope, std::unordered_map<core::SymbolRef, core::LocalVariable> &aliases)
        : ctx(ctx), inWhat(inWhat), target(target), loops(loops), scope(scope), rescueScope(rescueScope),
          aliases(aliases){};
};
} // namespace cfg
} // namespace ruby_typer
#endif // SRUBY_BUILDER_H
