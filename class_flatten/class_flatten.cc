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
// defintion) from a class or file into their own instruction sequence (or, if there is only one, simply move it out of
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
    ~ClassFlattenWalk() {
        ENFORCE(classes.empty());
        ENFORCE(classStack.empty());
    }

    ast::ExpressionPtr preTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        classStack.emplace_back(classes.size());
        classes.emplace_back();

        return tree;
    }

    ast::ExpressionPtr postTransformClassDef(core::Context ctx, ast::ExpressionPtr tree) {
        ENFORCE(!classStack.empty());
        ENFORCE(classes.size() > classStack.back());
        ENFORCE(classes[classStack.back()] == nullptr);

        auto *classDef = ast::cast_tree<ast::ClassDef>(tree);
        auto inits = extractClassInit(ctx, classDef);

        core::MethodRef sym;
        ast::ExpressionPtr replacement;
        if (classDef->symbol == core::Symbols::root()) {
            // Every file may have its own top-level code, so uniqify the names.
            //
            // NOTE(nelhage): In general, we potentially need to do this for
            // every class, since Ruby allows reopening classes. However, since
            // pay-server bans that behavior, this should be OK here.
            sym = ctx.state.lookupStaticInitForFile(ctx.file);

            // Skip emitting a place-holder for the root object.
            replacement = ast::MK::EmptyTree();
        } else {
            sym = ctx.state.lookupStaticInitForClass(classDef->symbol);

            // We only need a representation of the runtime definition of the class in the
            // containing static-init if the file is compiled; such a definition is just
            // noise otherwise.
            if (ctx.file.data(ctx).compiledLevel == core::CompiledLevel::True) {
                replacement = ast::MK::DefineTopClassOrModule(classDef->declLoc, classDef->symbol);
            } else {
                replacement = ast::MK::EmptyTree();
            }
        }
        ENFORCE(!sym.data(ctx)->arguments.empty(), "<static-init> method should already have a block arg symbol: {}",
                sym.show(ctx));
        ENFORCE(sym.data(ctx)->arguments.back().flags.isBlock, "Last argument symbol is not a block arg: {}",
                sym.show(ctx));

        // Synthesize a block argument for this <static-init> block. This is rather fiddly,
        // because we have to know exactly what invariants desugar and namer set up about
        // methods and block arguments before us.
        auto blkLoc = core::LocOffsets::none();
        core::LocalVariable blkLocalVar(core::Names::blkArg(), 0);
        ast::MethodDef::ARGS_store args;
        args.emplace_back(ast::make_expression<ast::Local>(blkLoc, blkLocalVar));

        auto init =
            ast::make_expression<ast::MethodDef>(classDef->declLoc, classDef->declLoc, sym, core::Names::staticInit(),
                                                 std::move(args), std::move(inits), ast::MethodDef::Flags());
        ast::cast_tree_nonnull<ast::MethodDef>(init).flags.isRewriterSynthesized = false;
        ast::cast_tree_nonnull<ast::MethodDef>(init).flags.isSelfMethod = true;

        classDef->rhs.emplace_back(std::move(init));

        classes[classStack.back()] = std::move(tree);
        classStack.pop_back();

        return replacement;
    };

    ast::ExpressionPtr addClasses(core::Context ctx, ast::ExpressionPtr tree) {
        if (classes.empty()) {
            ENFORCE(sortedClasses().empty());
            return tree;
        }
        if (classes.size() == 1 && ast::isa_tree<ast::EmptyTree>(tree)) {
            // It was only 1 class to begin with, put it back
            return std::move(sortedClasses()[0]);
        }

        auto insSeq = ast::cast_tree<ast::InsSeq>(tree);
        if (insSeq == nullptr) {
            ast::InsSeq::STATS_store stats;
            auto sorted = sortedClasses();
            stats.insert(stats.begin(), make_move_iterator(sorted.begin()), make_move_iterator(sorted.end()));
            return ast::MK::InsSeq(tree.loc(), std::move(stats), std::move(tree));
        }

        for (auto &clas : sortedClasses()) {
            ENFORCE(!!clas);
            insSeq->stats.emplace_back(std::move(clas));
        }
        return tree;
    }

private:
    vector<ast::ExpressionPtr> sortedClasses() {
        ENFORCE(classStack.empty());
        auto ret = std::move(classes);
        classes.clear();
        return ret;
    }

    // We flatten nested classes into a flat list. We want to sort
    // them by their starts, so that `class A; class B; end; end` --> `class A;
    // end; class B; end`.
    //
    // In order to make TreeMap work out, we can't remove them from the AST
    // until the `postTransform*` hook. Appending them to a list at that point
    // would result in an "bottom-up" ordering, so instead we store a stack of
    // "where does the next definition belong" into `classStack`
    // which we push onto in the `preTransform* hook, and pop from in the `postTransform` hook.
    vector<ast::ExpressionPtr> classes;
    vector<int> classStack;
};

ast::ParsedFile runOne(core::Context ctx, ast::ParsedFile tree) {
    ClassFlattenWalk flatten;
    tree.tree = ast::TreeMap::apply(ctx, flatten, std::move(tree.tree));
    tree.tree = flatten.addClasses(ctx, std::move(tree.tree));

    return tree;
}

} // namespace sorbet::class_flatten
