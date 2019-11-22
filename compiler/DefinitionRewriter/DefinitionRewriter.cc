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
                auto magic = ast::MK::Send1(loc, ast::MK::Unsafe(loc, ast::MK::Constant(loc, core::Symbols::root())),
                                            Names::defineTopClassOrModule(ctx), classDef->name->deepCopy());
                rootClassDef->rhs.insert(rootClassDef->rhs.begin() + i, move(magic));
                i++;
                continue;
            }

            auto methodDef = ast::cast_tree<ast::MethodDef>(stat.get());
            if (methodDef) {
                auto method = methodDef->isSelf() ? Names::defineMethodSingleton(ctx) : Names::defineMethod(ctx);
                auto magic = ast::MK::Send2(loc, ast::MK::Unsafe(loc, ast::MK::Constant(loc, core::Symbols::root())),
                                            method, ast::MK::Self(loc), ast::MK::Symbol(loc, methodDef->name));
                rootClassDef->rhs.insert(rootClassDef->rhs.begin() + i, move(magic));
                i++;
                continue;
            }
        }
        return rootClassDef;
    }

private:
    DefinitionRewriterWalker() = default;
};

void DefinitionRewriter::run(core::MutableContext &ctx, ast::ClassDef *klass) {
    DefinitionRewriterWalker definitionRewriterWalker;
    Names::init(ctx);
    unique_ptr<ast::ClassDef> uniqueClass(klass);
    auto ret = ast::TreeMap::apply(ctx, definitionRewriterWalker, std::move(uniqueClass));
    klass = static_cast<ast::ClassDef *>(ret.release());
    ENFORCE(klass);
}

} // namespace sorbet::compiler
