#ifndef SORBET_FLATTEN_H
#define SORBET_FLATTEN_H

#include "ast/ast.h"

namespace sorbet::flatten {

class FlattenWalk {
public:
    FlattenWalk();
    ~FlattenWalk();

    static std::vector<ast::ParsedFile> run(core::MutableContext ctx, std::vector<ast::ParsedFile> trees);

    std::unique_ptr<ast::Expression> addClasses(core::Context ctx, std::unique_ptr<ast::Expression> tree);
    std::unique_ptr<ast::Expression> addMethods(core::Context ctx, std::unique_ptr<ast::Expression> tree);
    ast::ClassDef::RHS_store addMethods(core::Context ctx, ast::ClassDef::RHS_store rhs);
    std::unique_ptr<ast::MethodDef> preTransformMethodDef(core::Context ctx, std::unique_ptr<ast::MethodDef> methodDef);
    std::unique_ptr<ast::Expression> postTransformMethodDef(core::Context ctx,
                                                            std::unique_ptr<ast::MethodDef> methodDef);

    std::unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx,
                                                        std::unique_ptr<ast::ClassDef> classDef);
    std::unique_ptr<ast::Expression> postTransformClassDef(core::Context ctx, std::unique_ptr<ast::ClassDef> classDef);

    struct Methods {
        std::vector<std::unique_ptr<ast::MethodDef>> methods;
        std::vector<int> stack;
        Methods() = default;
    };

private:
    bool isDefinition(core::Context, const std::unique_ptr<ast::Expression> &);
    std::unique_ptr<ast::Expression> extractClassInit(core::Context, std::unique_ptr<ast::ClassDef> &);

    std::vector<std::unique_ptr<ast::ClassDef>> sortedClasses();

    std::vector<std::unique_ptr<ast::MethodDef>> popCurMethodDefs();
    Methods &curMethodSet();
    void newMethodSet();
    void popCurMethodSet();

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

    std::vector<Methods> methodScopes;
    std::vector<std::unique_ptr<ast::ClassDef>> classes;
    std::vector<int> classStack;
};

} // namespace sorbet::flatten

#endif
