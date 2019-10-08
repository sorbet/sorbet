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
            }

            auto methodDef = ast::cast_tree<ast::MethodDef>(stat.get());
            if (methodDef) {
                auto magic = ast::MK::Send2(loc, ast::MK::Constant(loc, core::Symbols::root()), core::Names::magic(),
                                            rootClassDef->name->deepCopy(), ast::MK::Symbol(loc, methodDef->name));
                i++;
                rootClassDef->rhs.insert(rootClassDef->rhs.begin() + i, move(magic));
            }
        }
        return rootClassDef;
    }

private:
    DefinitionRewriterWalker() = default;
};

void DefinitionRewriter::run(core::MutableContext &ctx, ast::ClassDef *klass) {
    DefinitionRewriterWalker definitionRewriterWalker;
    ast::MethodDef::ARGS_store args;
    auto loc = klass->loc;
    args.emplace_back(ast::MK::RestArg(loc, ast::MK::Local(loc, core::Names::arg0())));
    klass->rhs.insert(klass->rhs.begin(), ast::MK::Method(loc,
                loc, core::Names::magic(), std::move(args), ast::MK::EmptyTree()));
    unique_ptr<ast::ClassDef> uniqueClass(klass);
    auto ret = ast::TreeMap::apply(ctx, definitionRewriterWalker, std::move(uniqueClass));
    klass = static_cast<ast::ClassDef *>(ret.release());
    ENFORCE(klass);
}

} // namespace sorbet::compiler
