#include "rewriter/Flatten.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/core.h"

#include <utility>

using namespace std;

// This DSL pass flattens nested methods, so that once we've reached a non-definition AST node (i.e. not a ClassDef or a
// MethodDef) then we know that there are no MethodDefs lurking deeper in the tree. In order to work correctly, this
// also needs to move some non-method-def things as well, specifically `sig`s and sends for method visibility
// (e.g. `private` and the like), and it also updates the static-ness of some MethodDefs based on where they have
// appeared in a nested context.
//
// So, a file like the following
//
// class A
//   sig{void}
//   private def foo
//     sig{void}
//     def self.bar; end
//   end
// end
//
// will morally be transformed into the following
//
// class A
//   sig{void}
//   private def foo; :bar; end
//   sig{void}
//   def bar; end   # notice the lack of `self.` here
// end
//
// So no nested methods exist any longer, and additionally, the nested method `bar` has had the `self.` qualifier
// removed: if you run the above code in Ruby, you'll find that `bar` is not defined as a class method on `A`, but
// rather as a not-always-available instance method on `A`, so introducing it as a static method is not at all
// correct. Finally, because methods evaluate to their corresponding symbols, the former location of `bar`'s definition
// has been replaced with the verbatim symbol `:bar`.
namespace sorbet::rewriter {

class FlattenWalk {
    enum class ScopeType { ClassScope, StaticMethodScope, InstanceMethodScope };

    struct ScopeInfo {
        // This tells us how many `class << self` levels we're nested inside
        u4 staticLevel;
        // this corresponds to the thing we're moving
        ScopeType scopeType;

        ScopeInfo(u4 staticLevel, ScopeType scopeType) : staticLevel(staticLevel), scopeType(scopeType) {}
    };

    // This is what we keep on the stack: we need to know whether an item should be moved or not (i.e. whether it's
    // nested or not) and keep a stack of the current 'staticness' level (i.e. how many levels of `def self.something`
    // we're inside)
    struct MethodData {
        // If this is non-nullopt, then it means that we've allocated space for an expression and this is the index to
        // that space. The reason we do this is so that we can keep the moved expressions in the same order we
        // originally see them in: in the following example
        //
        //   def foo
        //     sig{void}
        //     def bar
        //       sig{returns(Integer)}
        //       def baz; 1; end
        //     end
        //   end
        //
        // we want to end up with the following (modulo a few symbols):
        //
        //   def foo; end
        //   sig{void}
        //   def bar; end
        //   sig{returns(Integer)}
        //   def baz; 1; end
        //
        // which means we want `sig{void} `first, then `def bar`, then... and so forth. But we can only replace these
        // definitions in the post-traversal pass. The preTransform methods for the things which need to be moved will
        // always fire in the order that we want, but the postTransform methods won't: we'll get all the sigs in their
        // entirety before any postTransform invocation for a `methodDef`. So, this is a way of storing the order that
        // we want, and the `ClassScope` class below uses ENFORCEs to make sure that we always use these indices
        // correctly.
        optional<int> targetLocation;
        // this is all the metadata about this specific stack frame, and what scope it puts it into
        ScopeInfo scopeInfo;
        MethodData(optional<int> targetLocation, ScopeInfo scopeInfo)
            : targetLocation(targetLocation), scopeInfo(scopeInfo){};
    };

    // This represents something that needs to be moved to the end of the class scope as well as information about how
    // can reconstruct the preorder traversal)'static' it should be: i.e. whether it should have a `self.` qualifier and
    // whether it should be in a `class << self` block.
    struct MovedItem {
        ast::TreePtr expr;
        u4 staticLevel;
        MovedItem(ast::TreePtr expr, u4 staticLevel) : expr(move(expr)), staticLevel(staticLevel){};
        MovedItem() = default;
    };

    // This corresponds to a class scope, which in turn might have nested method scopes, as well as a queue of things to
    // move to the end of the class scope when we get to it.
    struct ClassScope {
        // this is the queue of methods that we want to move
        vector<MovedItem> moveQueue;
        // this is where we track the current state of which methods we're contained in
        vector<MethodData> stack;

        ClassScope() = default;

        // push a method scope, possibly noting whether
        void pushScope(ScopeInfo info) {
            stack.emplace_back(moveQueue.size(), info);
            moveQueue.emplace_back();
        }

        // Pop a method scope, and if the scope corresponded to something which we need to move then return the
        // information we need to move it. We expect that pushScope and popScope are called the same number of times,
        // and this should be exercised by ENFORCEs.
        optional<MethodData> popScope() {
            ENFORCE(stack.size() > 0);
            if (auto idx = stack.back().targetLocation) {
                // we have a non-nullopt target location, which means the element corresponding to this scope should get
                // moved to the end of the enclosing ClassDef
                // We should have allocated space in the move queue for this already
                ENFORCE(moveQueue.size() > *idx);
                // This space also should still be unoccupied
                ENFORCE(moveQueue[*idx].expr == nullptr);

                auto back = stack.back();
                stack.pop_back();
                return MethodData(back);
            } else {
                stack.pop_back();
                return std::nullopt;
            }
        }

        // this moves an expression to the already-allocated space
        void addExpr(MethodData md, ast::TreePtr expr) {
            ENFORCE(md.targetLocation);
            int idx = *md.targetLocation;
            ENFORCE(moveQueue.size() > idx);
            ENFORCE(moveQueue[idx].expr == nullptr);
            auto staticLevel = md.scopeInfo.staticLevel;
            // the staticLevel on the stack corresponded to how many `class << self` blocks we were inside of, which is
            // why we don't count `self.foo` methods as increasing the number there. We do want to treat them as static
            // once we replace them, so we add 1 to the computed staticLevel if we're moving a static method here.
            if (md.scopeInfo.scopeType == ScopeType::StaticMethodScope) {
                staticLevel += 1;
            }
            moveQueue[idx] = {move(expr), staticLevel};
        }
    };

    // each entry on this corresponds to a class or module scope in which we might in turn have nested methods. We only
    // care about the innermost class scope at a given time, but the outer class may still have definitions which we
    // need to move to the end, so we keep that below on the stack.
    vector<ClassScope> classScopes;

    // compute the new scope information.
    ScopeInfo computeScopeInfo(ScopeType scopeType) {
        auto &methods = curMethodSet();
        // if we're in instance scope, then we'll always stay in it. This is an incorrect approximation, but it's as
        // close as we can get with our specific model: in particular, if we have something like
        //
        // def foo; def bar; end; end
        //
        // then both #foo and #bar will be instance methods, but if we have
        //
        // def foo; def self.bar; end; end
        //
        // then #foo will be an instance method while #bar will be an instance method stored on the singleton class for
        // the instances on which #foo has been called: that is to say, it will act as an instance method, but it won't
        // be an instance method available on every instance of the enclosing class. We desugar it as though it's an
        // instance method.
        if (!methods.stack.empty() && methods.stack.back().scopeInfo.scopeType == ScopeType::InstanceMethodScope &&
            methods.stack.back().scopeInfo.staticLevel == 0) {
            return ScopeInfo(0, ScopeType::InstanceMethodScope);
        } else {
            // if we're not in an instance scope, then we carry on the staticLevel from the stack
            auto existingLevel = methods.stack.empty() ? 0 : methods.stack.back().scopeInfo.staticLevel;
            if (scopeType == ScopeType::ClassScope) {
                return ScopeInfo(existingLevel + 1, scopeType);
            } else {
                return ScopeInfo(existingLevel, scopeType);
            }
        }
    }

    void newMethodSet() {
        classScopes.emplace_back();
    }
    ClassScope &curMethodSet() {
        ENFORCE(!classScopes.empty());
        return classScopes.back();
    }
    void popCurMethodSet() {
        ENFORCE(!classScopes.empty());
        classScopes.pop_back();
    }

    // grab all the final moved items once we're done with everything: this will grab anything that needs to be moved at
    // the top level
    vector<MovedItem> popCurMethodDefs() {
        auto ret = std::move(curMethodSet().moveQueue);
        ENFORCE(curMethodSet().stack.empty());
        popCurMethodSet();
        return ret;
    };

    // extract all the methods from the current queue and put them at the end of the class's current body
    ast::ClassDef::RHS_store addClassDefMethods(core::Context ctx, ast::ClassDef::RHS_store rhs, core::Loc loc) {
        auto currentMethodDefs = popCurMethodDefs();
        // this adds all the methods at the appropriate 'staticness' level

        int highestLevel = 0;
        for (int i = 0; i < currentMethodDefs.size(); i++) {
            auto &expr = currentMethodDefs[i];
            if (highestLevel < expr.staticLevel) {
                highestLevel = expr.staticLevel;
            }
            // we need to make sure that we keep sends with their attached methods, so fix that up here
            if (i > 0) {
                auto send = ast::cast_tree<ast::Send>(currentMethodDefs[i - 1].expr);
                if (send != nullptr && send->fun == core::Names::sig()) {
                    currentMethodDefs[i - 1].staticLevel = expr.staticLevel;
                }
            }
        }

        // these will store the bodies of the `class << self` blocks we create at the end
        vector<ast::ClassDef::RHS_store> nestedClassBodies;
        for (int level = 2; level <= highestLevel; level++) {
            nestedClassBodies.emplace_back();
        }

        // This vector contains all the possible RHS_store target locations that we might move to.
        // Both vectors are indexed by staticLevel.
        vector<ast::ClassDef::RHS_store *> targets;

        // staticLevel 0 (no surrounding class << self) just goes into the top-level rhs.
        // do it while preserving order
        targets.emplace_back(&rhs);

        // staticLevel 1 (one surrounding class << self) also goes into the top-level rhs, but we want to interleave
        // them in source order so there's some special handling. nullptr here ensures that this fails early.
        targets.emplace_back(nullptr);

        // staticLevel 2 and up go into the to-be-created `class << self` blocks
        for (auto &target : nestedClassBodies) {
            targets.emplace_back(&target);
        }

        vector<vector<ast::TreePtr>> expressionsToBePutInTargets;
        // This makes each element be a length-0 vector
        expressionsToBePutInTargets.resize(targets.size());

        // move everything to its appropriate target
        for (auto &expr : currentMethodDefs) {
            if (auto methodDef = ast::cast_tree<ast::MethodDef>(expr.expr)) {
                methodDef->flags.isSelfMethod = expr.staticLevel > 0;
            }
            auto targetLevel = expr.staticLevel == 1 ? 0 : expr.staticLevel;
            expressionsToBePutInTargets[targetLevel].emplace_back(std::move(expr.expr));
        }

        int targetLevel = -1;
        for (auto &target : targets) {
            targetLevel += 1;
            if (targetLevel == 1) {
                ENFORCE(expressionsToBePutInTargets[targetLevel].empty());
            } else {
                target->insert(target->begin(), make_move_iterator(expressionsToBePutInTargets[targetLevel].begin()),
                               make_move_iterator(expressionsToBePutInTargets[targetLevel].end()));
            }
        }

        // generate the nested `class << self` blocks as needed and add them to the class
        for (auto &body : nestedClassBodies) {
            auto classDef = ast::MK::Class(loc.offsets(), loc,
                                           ast::make_tree<ast::UnresolvedIdent>(core::LocOffsets::none(),
                                                                                ast::UnresolvedIdent::Kind::Class,
                                                                                core::Names::singleton()),
                                           {}, std::move(body));
            rhs.emplace_back(std::move(classDef));
        }

        return rhs;
    }

public:
    FlattenWalk() {
        newMethodSet();
    }
    ~FlattenWalk() {
        ENFORCE(classScopes.empty());
    }

    ast::TreePtr preTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        if (!curMethodSet().stack.empty()) {
            curMethodSet().pushScope(computeScopeInfo(ScopeType::InstanceMethodScope));
        }
        newMethodSet();
        return tree;
    }

    ast::TreePtr postTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto &classDef = ast::ref_tree<ast::ClassDef>(tree);
        classDef.rhs =
            addClassDefMethods(ctx, std::move(classDef.rhs), core::Loc(classDef.declLoc.file(), classDef.loc));
        auto &methods = curMethodSet();
        if (curMethodSet().stack.empty()) {
            return tree;
        }
        // if this class is dirrectly nested inside a method, we want to steal it
        auto md = methods.popScope();
        ENFORCE(md);

        methods.addExpr(*md, move(tree));
        return ast::MK::EmptyTree();
    };

    ast::TreePtr preTransformSend(core::Context ctx, ast::TreePtr tree) {
        auto &send = ast::ref_tree<ast::Send>(tree);
        // we might want to move sigs, so we mostly use the same logic that we use for methods. The one exception is
        // that we don't know the 'staticness level' of a sig, as it depends on the method that follows it (whether that
        // method has a `self.` or not), so we'll fill that information in later
        if (send.fun == core::Names::sig()) {
            curMethodSet().pushScope(computeScopeInfo(ScopeType::StaticMethodScope));
        }

        return tree;
    }

    ast::TreePtr postTransformSend(core::Context ctx, ast::TreePtr tree) {
        auto &send = ast::ref_tree<ast::Send>(tree);
        auto &methods = curMethodSet();
        // if it's not a send, then we didn't make a stack frame for it
        if (send.fun != core::Names::sig()) {
            return tree;
        }
        // if we get a MethodData back, then we need to move this and replace it with an EmptyTree
        if (auto md = methods.popScope()) {
            methods.addExpr(*md, move(tree));
            return ast::MK::EmptyTree();
        }
        return tree;
    }

    ast::TreePtr preTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        auto &methodDef = ast::ref_tree<ast::MethodDef>(tree);
        // add a new scope for this method def
        curMethodSet().pushScope(computeScopeInfo(methodDef.flags.isSelfMethod ? ScopeType::StaticMethodScope
                                                                               : ScopeType::InstanceMethodScope));
        return tree;
    }

    ast::TreePtr postTransformMethodDef(core::Context ctx, ast::TreePtr tree) {
        auto &methodDef = ast::ref_tree<ast::MethodDef>(tree);
        auto &methods = curMethodSet();
        // if we get a MethodData back, then we need to move this and replace it
        auto md = methods.popScope();
        ENFORCE(md);

        // Stash some stuff from the methodDef before we move it
        auto loc = methodDef.declLoc;
        auto name = methodDef.name;
        auto keepName = methodDef.flags.isSelfMethod ? core::Names::keepSelfDef() : core::Names::keepDef();

        methods.addExpr(*md, move(tree));

        return ast::MK::Send2(loc.offsets(), ast::MK::Constant(loc.offsets(), core::Symbols::Sorbet_Private_Static()),
                              keepName, ast::MK::Self(loc.offsets()), ast::MK::Symbol(loc.offsets(), name));
    };

    ast::TreePtr addTopLevelMethods(core::Context ctx, ast::TreePtr tree) {
        auto &methods = curMethodSet().moveQueue;
        if (methods.empty()) {
            ENFORCE(popCurMethodDefs().empty());
            return tree;
        }
        if (methods.size() == 1 && ast::isa_tree<ast::EmptyTree>(tree)) {
            // It was only 1 method to begin with, put it back
            ast::TreePtr methodDef = std::move(popCurMethodDefs()[0].expr);
            return methodDef;
        }

        auto insSeq = ast::cast_tree<ast::InsSeq>(tree);
        if (insSeq == nullptr) {
            ast::InsSeq::STATS_store stats;
            tree = ast::make_tree<ast::InsSeq>(tree->loc, std::move(stats), std::move(tree));
            return addTopLevelMethods(ctx, std::move(tree));
        }

        for (auto &method : popCurMethodDefs()) {
            ENFORCE(method.expr != nullptr);
            insSeq->stats.emplace_back(std::move(method.expr));
        }
        return tree;
    }
};

ast::TreePtr Flatten::run(core::Context ctx, ast::TreePtr tree) {
    FlattenWalk flatten;
    tree = ast::TreeMap::apply(ctx, flatten, std::move(tree));
    tree = flatten.addTopLevelMethods(ctx, std::move(tree));

    return tree;
}

} // namespace sorbet::rewriter
