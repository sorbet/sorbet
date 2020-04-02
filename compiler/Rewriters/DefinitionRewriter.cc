#include "compiler/Rewriters/DefinitionRewriter.h"
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
            auto classDef = ast::cast_tree<ast::ClassDef>(stat.get());
            if (classDef) {
                // WARNING: this assumes that
                // https://github.com/sorbet/sorbet/blob/c8ee5f5c34d0ae6d184f6d385504004e9eeb05c8/flattener/flatten.cc#L77
                // is still in Sorbet.
                // this helps us decompose cases where a single file is reopened multiple times and allows us to know
                // which staticInit to call
                auto loc = core::LocOffsets{classDef->declLoc.beginPos(), classDef->declLoc.beginPos()};
                auto magic = ast::MK::Send1(loc, ast::MK::Unsafe(loc, ast::MK::Constant(loc, core::Symbols::root())),
                                            Names::defineTopClassOrModule(ctx), classDef->name->deepCopy());
                ast::cast_tree<ast::Send>(magic.get())->flags.isRewriterSynthesized = true;
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
