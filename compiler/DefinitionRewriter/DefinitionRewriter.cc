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
                auto magic = ast::MK::Send1(loc, ast::MK::Constant(loc, core::Symbols::root()), DefinitionRewriter::registerClass,
                                            classDef->name->deepCopy());
                i++;
                rootClassDef->rhs.insert(rootClassDef->rhs.begin() + i, move(magic));
                continue;
            }

            auto methodDef = ast::cast_tree<ast::MethodDef>(stat.get());
            if (methodDef) {
                auto magic = ast::MK::Send2(loc, ast::MK::Constant(loc, core::Symbols::root()), DefinitionRewriter::registerMethod,
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

void registerMagicMethods(core::MutableContext &ctx, ast::ClassDef *klass) {
    auto loc = klass->loc;
    DefinitionRewriter::registerClass = ctx.state.enterNameUTF8("<registerClass>");
    DefinitionRewriter::registerMethod = ctx.state.enterNameUTF8("<registerMethod>");

    ast::MethodDef::ARGS_store classArgs;
    classArgs.emplace_back(ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())));
    klass->rhs.insert(klass->rhs.begin(), ast::MK::Method(loc, loc, DefinitionRewriter::registerClass, std::move(classArgs), ast::MK::EmptyTree()));

    ast::MethodDef::ARGS_store methodArgs;
    methodArgs.emplace_back(ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())));
    klass->rhs.insert(klass->rhs.begin(), ast::MK::Method(loc, loc, DefinitionRewriter::registerMethod, std::move(methodArgs), ast::MK::EmptyTree()));
}

void DefinitionRewriter::run(core::MutableContext &ctx, ast::ClassDef *klass) {
    DefinitionRewriterWalker definitionRewriterWalker;
    registerMagicMethods(ctx, klass);
    unique_ptr<ast::ClassDef> uniqueClass(klass);
    auto ret = ast::TreeMap::apply(ctx, definitionRewriterWalker, std::move(uniqueClass));
    klass = static_cast<ast::ClassDef *>(ret.release());
    ENFORCE(klass);
}

core::NameRef DefinitionRewriter::registerClass;
core::NameRef DefinitionRewriter::registerMethod;

} // namespace sorbet::compiler
