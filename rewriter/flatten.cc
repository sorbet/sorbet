#include "rewriter/flatten.h"
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
        // we need to keep information around about whether we're in a static outer context: for example, if we have
        //
        //   def self.foo; def bar; end; end
        //
        // then we should flatten it to
        //
        //  def self.foo; end
        //  def self.bar; end
        //
        // which means when we get to `bar` we need to know that the outer context `foo` is static. We pass that down
        // the current stack by means of this `staticLevel` variable.
        int staticLevel;
        // this will be true if the thing we're moving here is
        //   class << self
        // and false otherwise
        bool isClassScope;
        MethodData(optional<int> targetLocation, int staticLevel, bool isClassScope)
            : targetLocation(targetLocation), staticLevel(staticLevel), isClassScope(isClassScope) {};
    };

    // This represents something that needs to be moved to the end of the class scope as well as information about how
    // can reconstruct the preorder traversal)'static' it should be: i.e. whether it should have a `self.` qualifier and
    // whether it should be in a `class << self` block.
    struct MovedItem {
        unique_ptr<ast::Expression> expr;
        int staticLevel;
        MovedItem(unique_ptr<ast::Expression> expr, int staticLevel) : expr(move(expr)), staticLevel(staticLevel){};
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
        void pushScope(int staticLevel, bool isClassScope) {
            if (stack.size() == 0 || stack.back().isClassScope) {
                // we're at the top level of a class, not nested inside a method, which means we don't need to move
                // anything: we'll add to the stack but don't need to allocate space on the move queue
                stack.emplace_back(std::nullopt, staticLevel, isClassScope);
            } else {
                // we're nested inside another method: that means that whatever scope we're recording is going to need
                // to be moved out, so we'll allocate space in the move queue and note the index of the allocated space
                // (so that we can reconstruct the original traversal order)
                stack.emplace_back(moveQueue.size(), staticLevel, isClassScope);
                moveQueue.emplace_back();
            }
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
        void addExpr(MethodData md, unique_ptr<ast::Expression> expr) {
            ENFORCE(md.targetLocation);
            int idx = *md.targetLocation;
            ENFORCE(moveQueue.size() > idx);
            ENFORCE(moveQueue[idx].expr == nullptr);
            moveQueue[idx] = {move(expr), md.staticLevel};
        }
    };

    // each entry on this corresponds to a class or module scope in which we might in turn have nested methods. We only
    // care about the innermost class scope at a given time, but the outer class may still have definitions which we
    // need to move to the end, so we keep that below on the stack.
    vector<ClassScope> classScopes;

    int computeStaticLevel(const ast::MethodDef &methodDef) {
        auto &methods = curMethodSet();
        int prevLevel = methods.stack.empty() ? 0 : methods.stack.back().staticLevel;
        return prevLevel + (methodDef.isSelf() ? 1 : 0);
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
        if (curMethodSet().moveQueue.size() == 1 && rhs.size() == 1 && ast::isa_tree<ast::EmptyTree>(rhs[0].get())) {
            // It was only 1 method to begin with, put it back
            rhs.pop_back();
            rhs.emplace_back(std::move(popCurMethodDefs()[0].expr));
            return rhs;
        }

        auto exprs = popCurMethodDefs();
        // TODO: remove all this
        //
        // this add the nested methods at the appropriate 'staticness level'

        int highestLevel = 0;
        for (int i = 0; i < exprs.size(); i++) {
            auto &expr = exprs[i];
            if (highestLevel < expr.staticLevel) {
                highestLevel = expr.staticLevel;
            }
            // we need to make sure that we keep sends with their attached methods, so fix that up here
            if (i > 0) {
                auto send = ast::cast_tree<ast::Send>(exprs[i - 1].expr.get());
                if (send != nullptr && send->fun == core::Names::sig()) {
                    exprs[i - 1].staticLevel = expr.staticLevel;
                }
            }
        }

        // these will store the bodies of the `class << self` blocks we create at the end
        vector<ast::ClassDef::RHS_store> nestedBlocks;
        for (int level = 2; level <= highestLevel; level++) {
            nestedBlocks.emplace_back();
        }

        // this vector contains all the possible RHS target locations that we might move to
        vector<ast::ClassDef::RHS_store *> targets;
        // 0 and 1 both go into the class itself
        targets.emplace_back(&rhs);
        targets.emplace_back(&rhs);
        // 2 and up go into the to-be-created `class << self` blocks
        for (auto &tgt : nestedBlocks) {
            targets.emplace_back(&tgt);
        }

        // move everything to its appropriate target
        for (auto &expr : exprs) {
            if (auto methodDef = ast::cast_tree<ast::MethodDef>(expr.expr.get())) {
                methodDef->setIsSelf(expr.staticLevel > 0);
            }
            targets[expr.staticLevel]->emplace_back(std::move(expr.expr));
        }

        // generate the nested `class << self` blocks as needed and add them to the class
        for (auto &body : nestedBlocks) {
            auto classDef =
                ast::MK::Class(loc, loc,
                               make_unique<ast::UnresolvedIdent>(core::Loc::none(), ast::UnresolvedIdent::Class,
                                                                 core::Names::singleton()),
                               {}, std::move(body), ast::ClassDefKind::Class);
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

    unique_ptr<ast::ClassDef> preTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> classDef) {
        if (auto ident = ast::cast_tree<ast::UnresolvedIdent>(classDef->name.get())) {
            ENFORCE(ident->name == core::Names::singleton());
            curMethodSet().pushScope(0, true);
            return classDef;
        }
        newMethodSet();
        return classDef;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> classDef) {
        auto &methods = curMethodSet();
        if (auto ident = ast::cast_tree<ast::UnresolvedIdent>(classDef->name.get())) {
            // if we get a MethodData back, then we need to move this and replace it with an EmptyTree
            if (auto md = methods.popScope()) {
                methods.addExpr(*md, move(classDef));
                return ast::MK::EmptyTree();
            }
            return classDef;
        }
        classDef->rhs = addClassDefMethods(ctx, std::move(classDef->rhs), classDef->loc);
        return classDef;
    };

    unique_ptr<ast::Send> preTransformSend(core::Context ctx, unique_ptr<ast::Send> send) {
        // we might want to move sigs, so we mostly use the same logic that we use for methods. The one exception is
        // that we don't know the 'staticness level' of a sig, as it depends on the method that follows it (whether that
        // method has a `self.` or not), so we'll fill that information in later
        if (send->fun == core::Names::sig()) {
            curMethodSet().pushScope(0, false);
        }

        return send;
    }

    unique_ptr<ast::Expression> postTransformSend(core::Context ctx, unique_ptr<ast::Send> send) {
        auto &methods = curMethodSet();
        // if it's not a send, then we didn't make a stack frame for it
        if (send->fun != core::Names::sig()) {
            return send;
        }
        // if we get a MethodData back, then we need to move this and replace it with an EmptyTree
        if (auto md = methods.popScope()) {
            methods.addExpr(*md, move(send));
            return ast::MK::EmptyTree();
        }
        return send;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> methodDef) {
        // add a new scope for this method def
        curMethodSet().pushScope(computeStaticLevel(*methodDef), false);
        return methodDef;
    }

    unique_ptr<ast::Expression> postTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> methodDef) {
        auto &methods = curMethodSet();
        // if we get a MethodData back, then we need to move this and replace it
        if (auto md = methods.popScope()) {
            // we'll replace MethodDefs with the symbol that corresponds to the name of the method
            auto replacement = ast::MK::Symbol(methodDef->loc, methodDef->name);
            methods.addExpr(*md, move(methodDef));
            return replacement;
        }
        return methodDef;
    };

    unique_ptr<ast::Expression> addTopLevelMethods(core::Context ctx, unique_ptr<ast::Expression> tree) {
        auto &methods = curMethodSet().moveQueue;
        if (methods.empty()) {
            ENFORCE(popCurMethodDefs().empty());
            return tree;
        }
        if (methods.size() == 1 && ast::isa_tree<ast::EmptyTree>(tree.get())) {
            // It was only 1 method to begin with, put it back
            unique_ptr<ast::Expression> methodDef = std::move(popCurMethodDefs()[0].expr);
            return methodDef;
        }

        auto insSeq = ast::cast_tree<ast::InsSeq>(tree.get());
        if (insSeq == nullptr) {
            ast::InsSeq::STATS_store stats;
            tree = make_unique<ast::InsSeq>(tree->loc, std::move(stats), std::move(tree));
            return addTopLevelMethods(ctx, std::move(tree));
        }

        for (auto &method : popCurMethodDefs()) {
            ENFORCE(method.expr != nullptr);
            insSeq->stats.emplace_back(std::move(method.expr));
        }
        return tree;
    }
};

unique_ptr<ast::Expression> Flatten::run(core::Context ctx, unique_ptr<ast::Expression> tree) {
    FlattenWalk flatten;
    tree = ast::TreeMap::apply(ctx, flatten, std::move(tree));
    tree = flatten.addTopLevelMethods(ctx, std::move(tree));

    return tree;
}

} // namespace sorbet::rewriter
