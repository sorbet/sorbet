#include "ast/ast.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "core/core.h"
#include "flatten/flatten.h"

#include <utility>

using namespace std;

namespace sorbet::flatten {

unique_ptr<ast::Expression> FlattenWalk::run(core::MutableContext ctx, unique_ptr<ast::Expression> tree) {
    FlattenWalk flatten;
    tree = ast::TreeMap::apply(ctx, flatten, std::move(tree));
    tree = flatten.addClasses(ctx, std::move(tree));
    tree = flatten.addMethods(ctx, std::move(tree));

    return tree;
}

bool FlattenWalk::isDefinition(core::Context ctx, const unique_ptr<ast::Expression> &what) {
    if (ast::isa_tree<ast::MethodDef>(what.get())) {
        return true;
    }
    if (ast::isa_tree<ast::ClassDef>(what.get())) {
        return true;
    }

    if (auto asgn = ast::cast_tree<ast::Assign>(what.get())) {
        return ast::isa_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
    }
    return false;
}

unique_ptr<ast::Expression> FlattenWalk::extractClassInit(core::Context ctx, unique_ptr<ast::ClassDef> &klass) {
    ast::InsSeq::STATS_store inits;


    for (auto it = klass->rhs.begin(); it != klass->rhs.end(); /* nothing */) {
        if (isDefinition(ctx, *it)) {
            ++it;
            continue;
        }
        inits.emplace_back(std::move(*it));
        it = klass->rhs.erase(it);
    }

    if (inits.empty()) {
        return nullptr;
    }
    if (inits.size() == 1) {
        return std::move(inits.front());
    }
    return make_unique<ast::InsSeq>(klass->declLoc, std::move(inits), make_unique<ast::EmptyTree>());
}

FlattenWalk::FlattenWalk() {
    newMethodSet();
}
FlattenWalk::~FlattenWalk() {
    ENFORCE(methodScopes.empty());
    ENFORCE(classes.empty());
    ENFORCE(classStack.empty());
}

unique_ptr<ast::ClassDef> FlattenWalk::preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> classDef) {
    newMethodSet();
    classStack.emplace_back(classes.size());
    classes.emplace_back();

    auto inits = extractClassInit(ctx, classDef);
    if (inits == nullptr) {
        return classDef;
    }

    core::SymbolRef sym;
    if (classDef->symbol == core::Symbols::root()) {
        // Every file may have its own top-level code, so uniqify the names.
        //
        // NOTE(nelhage): In general, we potentially need to do this for
        // every class, since Ruby allows reopening classes. However, since
        // pay-server bans that behavior, this should be OK here.
        sym = ctx.state.staticInitForFile(inits->loc);
    } else {
        sym = ctx.state.staticInitForClass(classDef->symbol, inits->loc);
    }
    ENFORCE(!sym.data(ctx)->arguments().empty(), "<static-init> method should already have a block arg symbol: {}",
            sym.data(ctx)->show(ctx));
    ENFORCE(sym.data(ctx)->arguments().back().data(ctx)->isBlockArgument(),
            "Last argument symbol is not a block arg: {}" + sym.data(ctx)->show(ctx));

    // Synthesize a block argument for this <static-init> block. This is rather fiddly,
    // because we have to know exactly what invariants desugar and namer set up about
    // methods and block arguments before us.
    auto blkLoc = core::Loc::none(inits->loc.file());
    core::LocalVariable blkLocalVar(core::Names::blkArg(), 0);
    ast::MethodDef::ARGS_store args;
    args.emplace_back(make_unique<ast::Local>(blkLoc, blkLocalVar));

    auto init = make_unique<ast::MethodDef>(inits->loc, inits->loc, sym, core::Names::staticInit(), std::move(args),
                                            std::move(inits), true);

    classDef->rhs.emplace_back(std::move(init));

    return classDef;
}

unique_ptr<ast::MethodDef> FlattenWalk::preTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> methodDef) {
    auto &methods = curMethodSet();
    methods.stack.emplace_back(methods.methods.size());
    methods.methods.emplace_back();
    return methodDef;
}

unique_ptr<ast::Expression> FlattenWalk::postTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> classDef) {
    ENFORCE(!classStack.empty());
    ENFORCE(classes.size() > classStack.back());
    ENFORCE(classes[classStack.back()] == nullptr);

    classDef->rhs = addMethods(ctx, std::move(classDef->rhs));
    classes[classStack.back()] = std::move(classDef);
    classStack.pop_back();
    return make_unique<ast::EmptyTree>();
};

unique_ptr<ast::Expression> FlattenWalk::postTransformMethodDef(core::Context ctx, unique_ptr<ast::MethodDef> methodDef) {
    auto &methods = curMethodSet();
    ENFORCE(!methods.stack.empty());
    ENFORCE(methods.methods.size() > methods.stack.back());
    ENFORCE(methods.methods[methods.stack.back()] == nullptr);

    methods.methods[methods.stack.back()] = std::move(methodDef);
    methods.stack.pop_back();
    return make_unique<ast::EmptyTree>();
};

unique_ptr<ast::Expression> FlattenWalk::addClasses(core::Context ctx, unique_ptr<ast::Expression> tree) {
    if (classes.empty()) {
        ENFORCE(sortedClasses().empty());
        return tree;
    }
    if (classes.size() == 1 && (ast::cast_tree<ast::EmptyTree>(tree.get()) != nullptr)) {
        // It was only 1 class to begin with, put it back
        return std::move(sortedClasses()[0]);
    }

    auto insSeq = ast::cast_tree<ast::InsSeq>(tree.get());
    if (insSeq == nullptr) {
        ast::InsSeq::STATS_store stats;
        auto sorted = sortedClasses();
        stats.insert(stats.begin(), make_move_iterator(sorted.begin()), make_move_iterator(sorted.end()));
        return ast::MK::InsSeq(tree->loc, std::move(stats), std::move(tree));
    }

    for (auto &clas : sortedClasses()) {
        ENFORCE(!!clas);
        insSeq->stats.emplace_back(std::move(clas));
    }
    return tree;
}

unique_ptr<ast::Expression> FlattenWalk::addMethods(core::Context ctx, unique_ptr<ast::Expression> tree) {
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

vector<unique_ptr<ast::ClassDef>> FlattenWalk::sortedClasses() {
    ENFORCE(classStack.empty());
    auto ret = std::move(classes);
    classes.clear();
    return ret;
}

ast::ClassDef::RHS_store FlattenWalk::addMethods(core::Context ctx, ast::ClassDef::RHS_store rhs) {
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

vector<unique_ptr<ast::MethodDef>> FlattenWalk::popCurMethodDefs() {
    auto ret = std::move(curMethodSet().methods);
    ENFORCE(curMethodSet().stack.empty());
    popCurMethodSet();
    return ret;
};

void FlattenWalk::newMethodSet() {
    methodScopes.emplace_back();
}
FlattenWalk::Methods &FlattenWalk::curMethodSet() {
    ENFORCE(!methodScopes.empty());
    return methodScopes.back();
}
void FlattenWalk::popCurMethodSet() {
    ENFORCE(!methodScopes.empty());
    methodScopes.pop_back();
}

}
