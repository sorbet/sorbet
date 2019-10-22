#include "dsl/flatten.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/concurrency/WorkerPool.h"
#include "core/core.h"

#include <utility>

using namespace std;

namespace sorbet::dsl {

class FlattenWalk {
private:
public:
    FlattenWalk() {
        newMethodSet();
    }
    ~FlattenWalk() {
        ENFORCE(methodScopes.empty());
        ENFORCE(classStack.empty());
    }

    unique_ptr<ast::ClassDef> preTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> classDef) {
        newMethodSet();
        int size = classStack.size();
        classStack.emplace_back(ClassScope{size, classDef->name.get()});
        return classDef;
    }

    unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> methodDef) {
        if (skipMethods.contains(methodDef.get())) {
            return methodDef;
        }
        auto &methods = curMethodSet();
        methods.stack.emplace_back(methods.methods.size());
        methods.methods.emplace_back();
        return methodDef;
    }

    /// Returns `true` if the method is one of the modifier names in Ruby (e.g. 'private' or 'protected' or
    /// similar). This does not need to know about `module_function` because we have already re-written it in a previous
    /// DSL pass.
    bool isMethodModifier(ast::Send& send) {
        auto fun = send.fun;
        return (fun == core::Names::private_() || fun == core::Names::protected_() || fun == core::Names::public_() ||
                fun == core::Names::privateClassMethod()) && send.args.size() == 1 && ast::isa_tree<ast::MethodDef>(send.args[0].get());
    }

    unique_ptr<ast::Send> preTransformSend(core::Context ctx, unique_ptr<ast::Send> send) {
        if (send->fun == core::Names::sig() || isMethodModifier(*send)) {
            auto &methods = curMethodSet();
            methods.stack.emplace_back(methods.methods.size());
            methods.methods.emplace_back();

            if (isMethodModifier(*send) && send->args.size() >= 1) {
                skipMethods.insert(send->args[0].get());
            }
        }
        return send;
    }

    unique_ptr<ast::Expression> postTransformSend(core::Context ctx, unique_ptr<ast::Send> send) {
        if (send->fun == core::Names::sig() || isMethodModifier(*send)) {
            auto &methods = curMethodSet();
            ENFORCE(!methods.stack.empty());
            ENFORCE(methods.methods.size() > methods.stack.back());
            ENFORCE(methods.methods[methods.stack.back()] == nullptr);

            methods.methods[methods.stack.back()] = std::move(send);
            methods.stack.pop_back();

            return make_unique<ast::EmptyTree>();
        } else {
            return send;
        }
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> classDef) {
        ENFORCE(!classStack.empty());
        classDef->rhs = addMethods(ctx, std::move(classDef->rhs));
        classStack.pop_back();
        return classDef;
    };

    unique_ptr<ast::Expression> postTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> methodDef) {
        // if this method is contained in a send like `private` or `protected`, then we should not move it, because
        // moving the send will do that for us
        if (skipMethods.contains(methodDef.get())) {
            return methodDef;
        }
        auto &methods = curMethodSet();
        ENFORCE(!methods.stack.empty());
        ENFORCE(methods.methods.size() > methods.stack.back());
        ENFORCE(methods.methods[methods.stack.back()] == nullptr);

        methods.methods[methods.stack.back()] = std::move(methodDef);
        methods.stack.pop_back();
        return make_unique<ast::EmptyTree>();
    };

    unique_ptr<ast::Expression> addMethods(core::Context ctx, unique_ptr<ast::Expression> tree) {
        auto &methods = curMethodSet().methods;
        if (methods.empty()) {
            ENFORCE(popCurMethodDefs().empty());
            return tree;
        }
        if (methods.size() == 1 && (ast::cast_tree<ast::EmptyTree>(tree.get()) != nullptr)) {
            // It was only 1 method to begin with, put it back
            unique_ptr<ast::Expression> methodDef = std::move(popCurMethodDefs()[0]);
            return methodDef;
        }

        auto insSeq = ast::cast_tree<ast::InsSeq>(tree.get());
        if (insSeq == nullptr) {
            ast::InsSeq::STATS_store stats;
            tree = make_unique<ast::InsSeq>(tree->loc, std::move(stats), std::move(tree));
            return addMethods(ctx, std::move(tree));
        }

        for (auto &method : popCurMethodDefs()) {
            ENFORCE(!!method);
            insSeq->stats.emplace_back(std::move(method));
        }
        return tree;
    }

private:
    ast::ClassDef::RHS_store addMethods(core::Context ctx, ast::ClassDef::RHS_store rhs) {
        if (curMethodSet().methods.size() == 1 && rhs.size() == 1 &&
            (ast::cast_tree<ast::EmptyTree>(rhs[0].get()) != nullptr)) {
            // It was only 1 method to begin with, put it back
            rhs.pop_back();
            rhs.emplace_back(std::move(popCurMethodDefs()[0]));
            return rhs;
        }
        for (auto &method : popCurMethodDefs()) {
            ENFORCE(method.get() != nullptr);
            rhs.emplace_back(std::move(method));
        }
        return rhs;
    }

    vector<unique_ptr<ast::Expression>> popCurMethodDefs() {
        auto ret = std::move(curMethodSet().methods);
        ENFORCE(curMethodSet().stack.empty());
        popCurMethodSet();
        return ret;
    };

    struct Methods {
        vector<unique_ptr<ast::Expression>> methods;
        vector<int> stack;
        Methods() = default;
    };
    void newMethodSet() {
        methodScopes.emplace_back();
    }
    Methods &curMethodSet() {
        ENFORCE(!methodScopes.empty());
        return methodScopes.back();
    }
    void popCurMethodSet() {
        ENFORCE(!methodScopes.empty());
        methodScopes.pop_back();
    }

    struct ClassScope {
        int index;
        ast::Expression *name;
        ClassScope(int index, ast::Expression *name) : index(index), name(name){};
    };

    // We flatten nested classes and methods into a flat list. We want to sort
    // them by their starts, so that `class A; class B; end; end` --> `class A;
    // end; class B; end`.
    //
    // In order to make TreeMap work out, we can't remove them from the AST
    // until the `postTransform*` hook. Appending them to a list at that point
    // would result in an "bottom-up" ordering, so instead we store a stack of
    // "where does the next definition belong" into `classStack` and
    // `methodScopes.stack`, which we push onto in the `preTransform* hook, and
    // pop from in the `postTransform` hook.

    vector<Methods> methodScopes;
    vector<ClassScope> classStack;
    UnorderedSet<ast::Expression *> skipMethods;
};

unique_ptr<ast::Expression> Flatten::patchFile(core::Context ctx, unique_ptr<ast::Expression> tree) {
    FlattenWalk flatten;
    tree = ast::TreeMap::apply(ctx, flatten, std::move(tree));
    tree = flatten.addMethods(ctx, std::move(tree));

    return tree;
}

} // namespace sorbet::dsl
