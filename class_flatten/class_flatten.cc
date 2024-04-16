#include "class_flatten/class_flatten.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "common/concurrency/WorkerPool.h"
#include "core/core.h"

#include <utility>

using namespace std;

namespace sorbet::class_flatten {

bool shouldExtract(core::Context ctx, const ast::ExpressionPtr &what) {
    if (ast::isa_tree<ast::MethodDef>(what)) {
        return false;
    }
    if (ast::isa_tree<ast::ClassDef>(what)) {
        return false;
    }
    if (ast::isa_tree<ast::EmptyTree>(what)) {
        return false;
    }

    if (auto *asgn = ast::cast_tree<ast::Assign>(what)) {
        return !ast::isa_tree<ast::UnresolvedConstantLit>(asgn->lhs);
    }

    return true;
}

// pull all the non-definitions (i.e. anything that's not a method definition, a class definition, or a constant
// definition) from a class or file into their own instruction sequence (or, if there is only one, simply move it out of
// the class body and return it.)
ast::ExpressionPtr extractClassInit(core::Context ctx, ast::ClassDef *klass) {
    ast::InsSeq::STATS_store inits;

    auto it = remove_if(klass->rhs.begin(), klass->rhs.end(), [&](ast::ExpressionPtr &expr) {
        bool b = shouldExtract(ctx, expr);
        if (b) {
            inits.emplace_back(std::move(expr));
        }
        return b;
    });
    klass->rhs.erase(it, klass->rhs.end());

    if (inits.empty()) {
        return ast::MK::EmptyTree();
    }
    if (inits.size() == 1) {
        return std::move(inits.front());
    }
    return ast::MK::InsSeq(klass->declLoc, std::move(inits), ast::MK::EmptyTree());
}

class ClassFlattenWalk {
private:
public:
    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto *classDef = ast::cast_tree<ast::ClassDef>(tree);
        auto inits = extractClassInit(ctx, classDef);

        core::MethodRef sym;
        auto replacement = ast::MK::EmptyTree();

        if (!ast::isa_tree<ast::EmptyTree>(inits)) {
            if (classDef->symbol == core::Symbols::root()) {
                // Every file may have its own top-level code, so uniqify the names.
                //
                // NOTE(nelhage): In general, we potentially need to do this for
                // every class, since Ruby allows reopening classes. However, since
                // pay-server bans that behavior, this should be OK here.
                sym = ctx.state.lookupStaticInitForFile(ctx.file);

                // Skip emitting a place-holder for the root object.
            } else {
                sym = ctx.state.lookupStaticInitForClass(classDef->symbol);

                // We only need a representation of the runtime definition of the class in the
                // containing static-init if the file is compiled; such a definition is just
                // noise otherwise.
                if (ctx.file.data(ctx).compiledLevel == core::CompiledLevel::True) {
                    replacement = ast::MK::DefineTopClassOrModule(classDef->declLoc, classDef->symbol);
                }
            }
            ENFORCE(!sym.data(ctx)->arguments.empty(),
                    "<static-init> method should already have a block arg symbol: {}", sym.show(ctx));
            ENFORCE(sym.data(ctx)->arguments.back().flags.isBlock, "Last argument symbol is not a block arg: {}",
                    sym.show(ctx));

            // Synthesize a block argument for this <static-init> block. This is rather fiddly,
            // because we have to know exactly what invariants desugar and namer set up about
            // methods and block arguments before us.
            auto blkLoc = core::LocOffsets::none();
            core::LocalVariable blkLocalVar(core::Names::blkArg(), 0);
            ast::MethodDef::ARGS_store args;
            args.emplace_back(ast::make_expression<ast::Local>(blkLoc, blkLocalVar));

            auto init = ast::make_expression<ast::MethodDef>(classDef->declLoc, classDef->declLoc, sym,
                                                             core::Names::staticInit(), std::move(args),
                                                             std::move(inits), ast::MethodDef::Flags());
            ast::cast_tree_nonnull<ast::MethodDef>(init).flags.isRewriterSynthesized = false;
            ast::cast_tree_nonnull<ast::MethodDef>(init).flags.isSelfMethod = true;

            classDef->rhs.emplace_back(std::move(init));
        }
    };
};

ast::ParsedFile runOne(core::Context ctx, ast::ParsedFile tree) {
    ClassFlattenWalk flatten;
    ast::TreeWalk::apply(ctx, flatten, tree.tree);

    return tree;
}

} // namespace sorbet::class_flatten
