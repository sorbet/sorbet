#include "rewriter/Mattr.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::rewriter {

static bool literalSymbolEqual(const core::GlobalState &gs, const ast::TreePtr &node, core::NameRef name) {
    if (auto lit = ast::cast_tree_const<ast::Literal>(node)) {
        return lit->isSymbol(gs) && lit->asSymbol(gs) == name;
    }
    return false;
}

static bool isLiteralFalse(const core::GlobalState &gs, const ast::TreePtr &node) {
    if (auto lit = ast::cast_tree_const<ast::Literal>(node)) {
        return lit->isFalse(gs);
    }
    return false;
}

static void addInstanceCounterPart(vector<ast::TreePtr> &sink, const ast::TreePtr &sig, const ast::TreePtr &def) {
    sink.emplace_back(sig->deepCopy());
    auto instanceDef = def->deepCopy();
    ENFORCE(ast::isa_tree<ast::MethodDef>(def));
    ast::cast_tree<ast::MethodDef>(instanceDef)->flags.isSelfMethod = false;
    ast::cast_tree<ast::MethodDef>(instanceDef)->flags.isRewriterSynthesized = true;
    sink.emplace_back(move(instanceDef));
}

vector<ast::TreePtr> Mattr::run(core::MutableContext ctx, const ast::Send *send, ast::ClassDef::Kind classDefKind) {
    vector<ast::TreePtr> empty;
    bool doReaders = false;
    bool doWriters = false;
    bool doPredicates = false;
    if (send->fun == core::Names::mattrReader() || send->fun == core::Names::cattrReader()) {
        doReaders = true;
    } else if (send->fun == core::Names::mattrWriter() || send->fun == core::Names::cattrWriter()) {
        doWriters = true;
    } else if (send->fun == core::Names::mattrAccessor() || send->fun == core::Names::cattrAccessor()) {
        doReaders = true;
        doWriters = true;
    } else if (classDefKind == ast::ClassDef::Kind::Class && send->fun == core::Names::classAttribute()) {
        doReaders = true;
        doWriters = true;
        doPredicates = true;
    } else {
        return empty;
    }

    bool instanceReader = true;
    bool instanceWriter = true;
    bool instancePredicate = true;
    auto symbolArgsBound = send->args.size();

    if (send->args.empty()) {
        return empty;
    }
    if (auto *options = ast::cast_tree_const<ast::Hash>(send->args.back())) {
        symbolArgsBound--;
        for (int i = 0; i < options->keys.size(); i++) {
            auto &key = options->keys[i];
            auto &value = options->values[i];
            if (literalSymbolEqual(ctx, key, core::Names::instanceReader()) && isLiteralFalse(ctx, value)) {
                instanceReader = false;
            }
            if (literalSymbolEqual(ctx, key, core::Names::instanceWriter()) && isLiteralFalse(ctx, value)) {
                instanceWriter = false;
            }
            if (literalSymbolEqual(ctx, key, core::Names::instancePredicate()) && isLiteralFalse(ctx, value)) {
                instancePredicate = false;
            }
            if (literalSymbolEqual(ctx, key, core::Names::instanceAccessor()) && isLiteralFalse(ctx, value)) {
                instanceReader = false;
                instanceWriter = false;
            }
        }
    }

    if (symbolArgsBound == 0) {
        return empty;
    }

    vector<ast::TreePtr> result;
    for (int i = 0; i < symbolArgsBound; i++) {
        auto *lit = ast::cast_tree_const<ast::Literal>(send->args[i]);
        if (!lit || !lit->isSymbol(ctx)) {
            return empty;
        }
        auto loc = lit->loc;
        if (doReaders) {
            auto sig = ast::MK::Sig0(loc, ast::MK::Untyped(loc));
            auto def =
                ast::MK::SyntheticMethod0(loc, core::Loc(ctx.file, loc), lit->asSymbol(ctx), ast::MK::EmptyTree());
            ast::cast_tree_nonnull<ast::MethodDef>(def).flags.isSelfMethod = true;
            if (instanceReader) {
                addInstanceCounterPart(result, sig, def);
            }
            result.emplace_back(move(sig));
            result.emplace_back(move(def));
        }
        if (doWriters) {
            auto sig = ast::MK::Sig1(loc, ast::MK::Symbol(loc, core::Names::arg0()), ast::MK::Untyped(loc),
                                     ast::MK::Untyped(loc));
            auto def = ast::MK::SyntheticMethod1(loc, core::Loc(ctx.file, loc), lit->asSymbol(ctx).addEq(ctx),
                                                 ast::MK::Local(loc, core::Names::arg0()), ast::MK::EmptyTree());
            ast::cast_tree_nonnull<ast::MethodDef>(def).flags.isSelfMethod = true;
            if (instanceWriter) {
                addInstanceCounterPart(result, sig, def);
            }
            result.emplace_back(move(sig));
            result.emplace_back(move(def));
        }
        if (doPredicates && instancePredicate) {
            // even though the option is called instance_predicate, setting it to false also prevents the class method
            // from being generated.
            auto sig = ast::MK::Sig0(
                loc, ast::MK::UnresolvedConstant(loc, ast::MK::T(loc), core::Names::Constants::Boolean()));
            auto def = ast::MK::SyntheticMethod0(loc, core::Loc(ctx.file, loc), lit->asSymbol(ctx).addQuestion(ctx),
                                                 ast::MK::False(loc));
            ast::cast_tree_nonnull<ast::MethodDef>(def).flags.isSelfMethod = true;
            if (instanceReader && instancePredicate) {
                addInstanceCounterPart(result, sig, def);
            }
            result.emplace_back(move(sig));
            result.emplace_back(move(def));
        }
    }
    return result;
}

} // namespace sorbet::rewriter
