#ifndef SORBET_AST_HELPERSS_H
#define SORBET_AST_HELPERSS_H

#include "ast/ast.h"
#include "core/Names.h"

namespace sorbet::ast {

class MK {
public:
    static std::unique_ptr<Expression> EmptyTree() {
        return std::make_unique<ast::EmptyTree>();
    }

    static std::unique_ptr<Block> Block(core::Loc loc, std::unique_ptr<Expression> body, MethodDef::ARGS_store args,
                                        core::SymbolRef symbol = core::Symbols::noSymbol()) {
        auto blk = std::make_unique<ast::Block>(loc, std::move(args), std::move(body));
        blk->symbol = symbol;
        return blk;
    }

    static std::unique_ptr<ast::Block> Block0(core::Loc loc, std::unique_ptr<Expression> body) {
        MethodDef::ARGS_store args;
        return Block(loc, std::move(body), std::move(args));
    }

    static std::unique_ptr<Expression> Send(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                            Send::ARGS_store args, u4 flags = 0,
                                            std::unique_ptr<ast::Block> blk = nullptr) {
        auto send = std::make_unique<ast::Send>(loc, std::move(recv), fun, std::move(args), std::move(blk));
        send->flags = flags;
        return send;
    }

    static std::unique_ptr<Literal> Literal(core::Loc loc, const core::TypePtr &tpe) {
        auto lit = std::make_unique<ast::Literal>(loc, tpe);
        return lit;
    }

    static std::unique_ptr<Expression> Return(core::Loc loc, std::unique_ptr<Expression> expr) {
        return std::make_unique<ast::Return>(loc, std::move(expr));
    }

    static std::unique_ptr<Expression> Next(core::Loc loc, std::unique_ptr<Expression> expr) {
        return std::make_unique<ast::Next>(loc, std::move(expr));
    }

    static std::unique_ptr<Expression> Break(core::Loc loc, std::unique_ptr<Expression> expr) {
        return std::make_unique<ast::Break>(loc, std::move(expr));
    }

    static std::unique_ptr<Expression> Send0(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun) {
        Send::ARGS_store nargs;
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static std::unique_ptr<Expression> Send1(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                             std::unique_ptr<Expression> arg1) {
        Send::ARGS_store nargs;
        nargs.emplace_back(std::move(arg1));
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static std::unique_ptr<Expression> Send2(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                             std::unique_ptr<Expression> arg1, std::unique_ptr<Expression> arg2) {
        Send::ARGS_store nargs;
        nargs.emplace_back(std::move(arg1));
        nargs.emplace_back(std::move(arg2));
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static std::unique_ptr<Expression> Send3(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                             std::unique_ptr<Expression> arg1, std::unique_ptr<Expression> arg2,
                                             std::unique_ptr<Expression> arg3) {
        Send::ARGS_store nargs;
        nargs.emplace_back(std::move(arg1));
        nargs.emplace_back(std::move(arg2));
        nargs.emplace_back(std::move(arg3));
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static std::unique_ptr<Expression> Nil(core::Loc loc) {
        return std::make_unique<ast::Literal>(loc, core::Types::nilClass());
    }

    static std::unique_ptr<ConstantLit> Constant(core::Loc loc, core::SymbolRef symbol) {
        ENFORCE(symbol.exists());
        return std::make_unique<ast::ConstantLit>(loc, symbol, nullptr);
    }

    static std::unique_ptr<Reference> Local(core::Loc loc, core::NameRef name) {
        return std::make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Local, name);
    }

    static std::unique_ptr<Reference> OptionalArg(core::Loc loc, std::unique_ptr<Reference> inner,
                                                  std::unique_ptr<Expression> default_) {
        return std::make_unique<ast::OptionalArg>(loc, std::move(inner), std::move(default_));
    }

    static std::unique_ptr<Reference> KeywordArg(core::Loc loc, std::unique_ptr<Reference> inner) {
        return std::make_unique<ast::KeywordArg>(loc, std::move(inner));
    }

    static std::unique_ptr<Reference> RestArg(core::Loc loc, std::unique_ptr<Reference> inner) {
        return std::make_unique<ast::RestArg>(loc, std::move(inner));
    }

    static std::unique_ptr<Reference> Instance(core::Loc loc, core::NameRef name) {
        return std::make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Instance, name);
    }

    static std::unique_ptr<Expression> cpRef(Reference &name) {
        if (auto *nm = cast_tree<UnresolvedIdent>(&name)) {
            return std::make_unique<UnresolvedIdent>(name.loc, nm->kind, nm->name);
        }
        Exception::notImplemented();
    }

    static std::unique_ptr<Expression> Assign(core::Loc loc, std::unique_ptr<Expression> lhs,
                                              std::unique_ptr<Expression> rhs) {
        if (auto *s = cast_tree<ast::Send>(lhs.get())) {
            s->args.emplace_back(std::move(rhs));
            return lhs;
        }

        return std::make_unique<ast::Assign>(loc, std::move(lhs), std::move(rhs));
    }

    static std::unique_ptr<Expression> Assign(core::Loc loc, core::NameRef name, std::unique_ptr<Expression> rhs) {
        return Assign(loc, Local(loc, name), std::move(rhs));
    }

    static std::unique_ptr<Expression> If(core::Loc loc, std::unique_ptr<Expression> cond,
                                          std::unique_ptr<Expression> thenp, std::unique_ptr<Expression> elsep) {
        return std::make_unique<ast::If>(loc, std::move(cond), std::move(thenp), std::move(elsep));
    }

    static std::unique_ptr<Expression> While(core::Loc loc, std::unique_ptr<Expression> cond,
                                             std::unique_ptr<Expression> body) {
        return std::make_unique<ast::While>(loc, std::move(cond), std::move(body));
    }

    static std::unique_ptr<Expression> Self(core::Loc loc) {
        return std::make_unique<ast::Local>(loc, core::LocalVariable::selfVariable());
    }

    static std::unique_ptr<Expression> InsSeq(core::Loc loc, InsSeq::STATS_store stats,
                                              std::unique_ptr<Expression> expr) {
        if (!stats.empty()) {
            return std::make_unique<ast::InsSeq>(loc, std::move(stats), std::move(expr));
        }
        return expr;
    }

    static std::unique_ptr<Expression> Splat(core::Loc loc, std::unique_ptr<Expression> arg) {
        auto to_a = Send0(loc, std::move(arg), core::Names::to_a());
        return Send1(loc, Constant(loc, core::Symbols::Magic()), core::Names::splat(), std::move(to_a));
    }

    static std::unique_ptr<Expression> InsSeq1(core::Loc loc, std::unique_ptr<Expression> stat,
                                               std::unique_ptr<Expression> expr) {
        InsSeq::STATS_store stats;
        stats.emplace_back(std::move(stat));
        return InsSeq(loc, std::move(stats), std::move(expr));
    }

    static std::unique_ptr<Expression> True(core::Loc loc) {
        return std::make_unique<ast::Literal>(loc, core::Types::trueClass());
    }

    static std::unique_ptr<Expression> False(core::Loc loc) {
        return std::make_unique<ast::Literal>(loc, core::Types::falseClass());
    }

    static std::unique_ptr<UnresolvedConstantLit> UnresolvedConstant(core::Loc loc, std::unique_ptr<Expression> scope,
                                                                     core::NameRef name) {
        return std::make_unique<ast::UnresolvedConstantLit>(loc, std::move(scope), name);
    }

    static std::unique_ptr<Expression> Int(core::Loc loc, int64_t val) {
        return std::make_unique<ast::Literal>(loc, core::make_type<core::LiteralType>(val));
    }

    static std::unique_ptr<Expression> Float(core::Loc loc, double val) {
        return std::make_unique<ast::Literal>(loc, core::make_type<core::LiteralType>(val));
    }

    static std::unique_ptr<Expression> Symbol(core::Loc loc, core::NameRef name) {
        return std::make_unique<ast::Literal>(loc, core::make_type<core::LiteralType>(core::Symbols::Symbol(), name));
    }

    static std::unique_ptr<Expression> String(core::Loc loc, core::NameRef value) {
        return std::make_unique<ast::Literal>(loc, core::make_type<core::LiteralType>(core::Symbols::String(), value));
    }

    static std::unique_ptr<MethodDef> Method(core::Loc loc, core::Loc declLoc, core::NameRef name,
                                             MethodDef::ARGS_store args, std::unique_ptr<Expression> rhs,
                                             u4 flags = 0) {
        if (args.empty() || (!isa_tree<ast::Local>(args.back().get()) && !isa_tree<BlockArg>(args.back().get()))) {
            auto blkLoc = core::Loc::none(declLoc.file());
            args.emplace_back(std::make_unique<BlockArg>(blkLoc, MK::Local(blkLoc, core::Names::blkArg())));
        }
        return std::make_unique<MethodDef>(loc, declLoc, core::Symbols::todo(), name, std::move(args), std::move(rhs),
                                           flags);
    }

    static std::unique_ptr<Expression> Method0(core::Loc loc, core::Loc declLoc, core::NameRef name,
                                               std::unique_ptr<Expression> rhs, u4 flags = 0) {
        MethodDef::ARGS_store args;
        return Method(loc, declLoc, name, std::move(args), std::move(rhs), flags);
    }

    static std::unique_ptr<Expression> Method1(core::Loc loc, core::Loc declLoc, core::NameRef name,
                                               std::unique_ptr<Expression> arg0, std::unique_ptr<Expression> rhs,
                                               u4 flags = 0) {
        MethodDef::ARGS_store args;
        args.emplace_back(std::move(arg0));
        return Method(loc, declLoc, name, std::move(args), std::move(rhs), flags);
    }

    static std::unique_ptr<ClassDef> Class(core::Loc loc, core::Loc declLoc, std::unique_ptr<Expression> name,
                                           ClassDef::ANCESTORS_store ancestors, ClassDef::RHS_store rhs,
                                           ClassDefKind kind) {
        return std::make_unique<ClassDef>(loc, declLoc, core::Symbols::todo(), std::move(name), std::move(ancestors),
                                          std::move(rhs), kind);
    }

    static std::unique_ptr<Expression> Array(core::Loc loc, Array::ENTRY_store entries) {
        return std::make_unique<ast::Array>(loc, std::move(entries));
    }

    static std::unique_ptr<Expression> Hash(core::Loc loc, Hash::ENTRY_store keys, Hash::ENTRY_store values) {
        return std::make_unique<ast::Hash>(loc, std::move(keys), std::move(values));
    }

    static std::unique_ptr<Expression> Hash0(core::Loc loc) {
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        return Hash(loc, std::move(keys), std::move(values));
    }

    static std::unique_ptr<Expression> Hash1(core::Loc loc, std::unique_ptr<Expression> key,
                                             std::unique_ptr<Expression> value) {
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        keys.emplace_back(std::move(key));
        values.emplace_back(std::move(value));
        return Hash(loc, std::move(keys), std::move(values));
    }

    static std::unique_ptr<Expression> Sig(core::Loc loc, std::unique_ptr<Expression> hash,
                                           std::unique_ptr<Expression> ret) {
        auto params = Send1(loc, Self(loc), core::Names::params(), std::move(hash));
        auto returns = Send1(loc, std::move(params), core::Names::returns(), std::move(ret));
        auto sig = Send0(loc, Constant(loc, core::Symbols::Sorbet()), core::Names::sig());
        auto sigSend = ast::cast_tree<ast::Send>(sig.get());
        sigSend->block = Block0(loc, std::move(returns));
        return sig;
    }

    static std::unique_ptr<Expression> Sig0(core::Loc loc, std::unique_ptr<Expression> ret) {
        auto returns = Send1(loc, Self(loc), core::Names::returns(), std::move(ret));
        auto sig = Send0(loc, Constant(loc, core::Symbols::Sorbet()), core::Names::sig());
        auto sigSend = ast::cast_tree<ast::Send>(sig.get());
        sigSend->block = Block0(loc, std::move(returns));
        return sig;
    }

    static std::unique_ptr<Expression> Sig1(core::Loc loc, std::unique_ptr<Expression> key,
                                            std::unique_ptr<Expression> value, std::unique_ptr<Expression> ret) {
        return Sig(loc, Hash1(loc, std::move(key), std::move(value)), std::move(ret));
    }

    static std::unique_ptr<Expression> Cast(core::Loc loc, std::unique_ptr<Expression> type) {
        if (auto *send = cast_tree<ast::Send>(type.get())) {
            if (send->fun == core::Names::untyped()) {
                return Unsafe(loc, Nil(loc));
            }
        }
        return Send2(loc, Constant(loc, core::Symbols::T()), core::Names::cast(), Unsafe(loc, Nil(loc)),
                     std::move(type));
    }

    static std::unique_ptr<Expression> T(core::Loc loc) {
        return Constant(loc, core::Symbols::T());
    }

    static std::unique_ptr<Expression> Let(core::Loc loc, std::unique_ptr<Expression> value,
                                           std::unique_ptr<Expression> type) {
        return Send2(loc, T(loc), core::Names::let(), std::move(value), std::move(type));
    }

    static std::unique_ptr<Expression> Unsafe(core::Loc loc, std::unique_ptr<Expression> inner) {
        return Send1(loc, T(loc), core::Names::unsafe(), std::move(inner));
    }

    static std::unique_ptr<Expression> Untyped(core::Loc loc) {
        return Send0(loc, T(loc), core::Names::untyped());
    }

    static std::unique_ptr<Expression> Nilable(core::Loc loc, std::unique_ptr<Expression> arg) {
        return Send1(loc, T(loc), core::Names::nilable(), std::move(arg));
    }

    static std::unique_ptr<Expression> KeepForIDE(std::unique_ptr<Expression> arg) {
        auto loc = core::Loc::none(arg->loc.file());
        return Send1(loc, Constant(loc, core::Symbols::RubyTyper()), core::Names::keepForIde(), std::move(arg));
    }

    static std::unique_ptr<Expression> KeepForTypechecking(std::unique_ptr<Expression> arg) {
        auto loc = core::Loc::none(arg->loc.file());
        return Send1(loc, Constant(loc, core::Symbols::RubyTyper()), core::Names::keepForTypechecking(),
                     std::move(arg));
    }

    static class Local *arg2Local(Expression *arg) {
        while (true) {
            if (auto *local = cast_tree<class Local>(arg)) {
                // Buried deep within every argument is a Local
                return local;
            }

            // Recurse into structure to find the Local
            typecase(arg, [&](class RestArg *rest) { arg = rest->expr.get(); },
                     [&](class KeywordArg *kw) { arg = kw->expr.get(); },
                     [&](class OptionalArg *opt) { arg = opt->expr.get(); },
                     [&](BlockArg *blk) { arg = blk->expr.get(); },
                     [&](ShadowArg *shadow) { arg = shadow->expr.get(); },
                     // ENFORCES are last so that we don't pay the price of casting in the happy path.
                     [&](UnresolvedIdent *opt) { ENFORCE(false, "Namer should have created a Local for this arg."); },
                     [&](Expression *expr) { ENFORCE(false, "Unexpected node type in argument position."); });
        }
    }
};

} // namespace sorbet::ast

#endif // SORBET_TREES_H
