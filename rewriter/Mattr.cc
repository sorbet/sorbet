#include "rewriter/Mattr.h"
#include "ast/Helpers.h"
#include "core/GlobalState.h"

using namespace std;

namespace sorbet::rewriter {

static bool literalSymbolEqual(const core::GlobalState &gs, ast::Expression *node, core::NameRef name) {
    if (auto lit = ast::cast_tree<ast::Literal>(node)) {
        return lit->isSymbol(gs) && lit->asSymbol(gs) == name;
    }
    return false;
}

static bool isLiteralFalse(const core::GlobalState &gs, ast::Expression *node) {
    if (auto lit = ast::cast_tree<ast::Literal>(node)) {
        return lit->isFalse(gs);
    }
    return false;
}

static void addInstanceCounterPart(vector<unique_ptr<ast::Expression>> &sink, const unique_ptr<ast::Expression> &sig,
                                   const unique_ptr<ast::MethodDef> &def) {
    sink.emplace_back(sig->deepCopy());
    auto instanceDef = def->deepCopy();
    ENFORCE(ast::isa_tree<ast::MethodDef>(def.get()));
    ast::cast_tree<ast::MethodDef>(instanceDef.get())->flags = ast::MethodDef::Flags::RewriterSynthesized;
    sink.emplace_back(move(instanceDef));
}

vector<unique_ptr<ast::Expression>> Mattr::run(core::MutableContext ctx, const ast::Send *send,
                                               ast::ClassDef::Kind classDefKind) {
    vector<unique_ptr<ast::Expression>> empty;
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
    if (auto options = ast::cast_tree<ast::Hash>(send->args.back().get())) {
        symbolArgsBound--;
        for (int i = 0; i < options->keys.size(); i++) {
            auto key = options->keys[i].get();
            auto value = options->values[i].get();
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

    vector<unique_ptr<ast::Expression>> result;
    for (int i = 0; i < symbolArgsBound; i++) {
        auto lit = ast::cast_tree<ast::Literal>(send->args[i].get());
        if (!lit || !lit->isSymbol(ctx)) {
            return empty;
        }
        auto loc = lit->loc;
        if (doReaders) {
            auto sig = ast::MK::Sig0(loc, ast::MK::Untyped(loc));
            auto def = ast::MK::Method0(loc, loc, lit->asSymbol(ctx), ast::MK::EmptyTree(),
                                        ast::MethodDef::Flags::SelfMethod | ast::MethodDef::Flags::RewriterSynthesized);
            if (instanceReader) {
                addInstanceCounterPart(result, sig, def);
            }
            result.emplace_back(move(sig));
            result.emplace_back(move(def));
        }
        if (doWriters) {
            auto sig = ast::MK::Sig1(loc, ast::MK::Symbol(loc, core::Names::arg0()), ast::MK::Untyped(loc),
                                     ast::MK::Untyped(loc));
            auto def = ast::MK::Method1(loc, loc, lit->asSymbol(ctx).addEq(ctx),
                                        ast::MK::Local(loc, core::Names::arg0()), ast::MK::EmptyTree(),
                                        ast::MethodDef::Flags::SelfMethod | ast::MethodDef::Flags::RewriterSynthesized);
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
            auto def = ast::MK::Method0(loc, loc, lit->asSymbol(ctx).addQuestion(ctx), ast::MK::False(loc),
                                        ast::MethodDef::Flags::SelfMethod | ast::MethodDef::Flags::RewriterSynthesized);
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
