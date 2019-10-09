#include "compiler/DefinitionRewriter/DefinitionRewriter.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"

using namespace std;

namespace sorbet::compiler {

class DefinitionRewriterWalker {
    friend class DefinitionRewriter;

public:
    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx,
                                                      unique_ptr<ast::ClassDef> rootClassDef) {
        for (auto i = 0; i < rootClassDef->rhs.size(); i++) {
            auto &stat = rootClassDef->rhs[i];
            auto loc = stat->loc;
            auto classDef = ast::cast_tree<ast::ClassDef>(stat.get());
            if (classDef) {
                auto magic = ast::MK::Send1(loc, ast::MK::Constant(loc, core::Symbols::root()), core::Names::magic(),
                                            classDef->name->deepCopy());
                i++;
                rootClassDef->rhs.insert(rootClassDef->rhs.begin() + i, move(magic));
                continue;
            }

            auto methodDef = ast::cast_tree<ast::MethodDef>(stat.get());
            if (methodDef) {
                auto magic = ast::MK::Send2(loc, ast::MK::Constant(loc, core::Symbols::root()), core::Names::magic(),
                                            rootClassDef->name->deepCopy(), ast::MK::Symbol(loc, methodDef->name));
                i++;
                rootClassDef->rhs.insert(rootClassDef->rhs.begin() + i, move(magic));
                continue;
            }
        }
        return rootClassDef;
    }

private:
    DefinitionRewriterWalker() = default;
};

void defineMagic(ast::ClassDef *klass) {
    ast::MethodDef::ARGS_store args;
    auto loc = klass->loc;
    args.emplace_back(ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())));
    klass->rhs.insert(klass->rhs.begin(),
                      ast::MK::Method(loc, loc, core::Names::magic(), std::move(args), ast::MK::EmptyTree()));
}

// This is really a deficiency in Sorbet's AST and should probably be upstreamed
// into the desugarer
unique_ptr<ast::ClassDef> wrapInClass(ast::ClassDef *klass) {
    if (ast::isa_tree<ast::EmptyTree>(klass->name.get())) {
        return unique_ptr<ast::ClassDef>(klass);
    }
    auto loc = klass->loc;
    ast::ClassDef::ANCESTORS_store ancestors;
    ast::ClassDef::RHS_store rhs;
    rhs.emplace_back(klass);
    return ast::MK::Class(loc, loc, ast::MK::EmptyTree(), std::move(ancestors), std::move(rhs),
                          ast::ClassDefKind::Module);
}

void DefinitionRewriter::run(core::MutableContext &ctx, ast::ClassDef *klass) {
    auto uniqueClass = wrapInClass(klass);
    defineMagic(uniqueClass.get());

    DefinitionRewriterWalker definitionRewriterWalker;
    auto ret = ast::TreeMap::apply(ctx, definitionRewriterWalker, std::move(uniqueClass));
    klass = static_cast<ast::ClassDef *>(ret.release());
    ENFORCE(klass);
}

} // namespace sorbet::compiler
