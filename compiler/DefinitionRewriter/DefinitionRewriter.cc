#include "compiler/DefinitionRewriter/DefinitionRewriter.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "compiler/Names/Names.h"

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
                auto magic = ast::MK::Send1(loc, ast::MK::Constant(loc, core::Symbols::root()),
                                            Names::sorbet_defineTopLevelClass, classDef->name->deepCopy());
                i++;
                rootClassDef->rhs.insert(rootClassDef->rhs.begin() + i, move(magic));
                continue;
            }

            auto methodDef = ast::cast_tree<ast::MethodDef>(stat.get());
            if (methodDef) {
                auto magic =
                    ast::MK::Send2(loc, ast::MK::Constant(loc, core::Symbols::root()), Names::sorbet_defineMethod,
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

void registerMagicMethod(core::MutableContext &ctx, ast::ClassDef *klass, core::NameRef name) {
    auto loc = klass->loc;
    ast::MethodDef::ARGS_store args;
    args.emplace_back(ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())));
    klass->rhs.insert(klass->rhs.begin(), ast::MK::Method(loc, loc, name, std::move(args), ast::MK::EmptyTree()));
}

void registerMagicMethods(core::MutableContext &ctx, ast::ClassDef *klass) {
    registerMagicMethod(ctx, klass, Names::sorbet_defineTopLevelModule);
    registerMagicMethod(ctx, klass, Names::sorbet_defineNestedModule);
    registerMagicMethod(ctx, klass, Names::sorbet_defineTopLevelClass);
    registerMagicMethod(ctx, klass, Names::sorbet_defineNestedClass);
    registerMagicMethod(ctx, klass, Names::sorbet_defineMethod);
    registerMagicMethod(ctx, klass, Names::sorbet_defineMethodSingleton);
}

void DefinitionRewriter::run(core::MutableContext &ctx, ast::ClassDef *klass) {
    DefinitionRewriterWalker definitionRewriterWalker;
    Names::init(ctx);
    registerMagicMethods(ctx, klass);
    unique_ptr<ast::ClassDef> uniqueClass(klass);
    auto ret = ast::TreeMap::apply(ctx, definitionRewriterWalker, std::move(uniqueClass));
    klass = static_cast<ast::ClassDef *>(ret.release());
    ENFORCE(klass);
}

} // namespace sorbet::compiler
