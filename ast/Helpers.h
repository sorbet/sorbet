#ifndef SORBET_AST_HELPERSS_H
#define SORBET_AST_HELPERSS_H

#include "ast/ast.h"
#include "core/Names.h"

namespace sorbet::ast {

class MK {
public:
    static std::unique_ptr<Expression> EmptyTree(core::Loc loc) {
        return std::make_unique<ast::EmptyTree>(loc);
    }

    static std::unique_ptr<Block> Block(core::Loc loc, std::unique_ptr<Expression> body, MethodDef::ARGS_store args,
                                        core::SymbolRef symbol = core::Symbols::noSymbol()) {
        auto blk = std::make_unique<ast::Block>(loc, move(args), move(body));
        blk->symbol = symbol;
        return blk;
    }

    static std::unique_ptr<ast::Block> Block0(core::Loc loc, std::unique_ptr<Expression> body) {
        MethodDef::ARGS_store args;
        return Block(loc, move(body), move(args));
    }

    static std::unique_ptr<Expression> Send(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                            Send::ARGS_store args, u4 flags = 0,
                                            std::unique_ptr<ast::Block> blk = nullptr) {
        auto send = std::make_unique<ast::Send>(loc, move(recv), fun, move(args), move(blk));
        send->flags = flags;
        return send;
    }

    static std::unique_ptr<Literal> Literal(core::Loc loc, const std::shared_ptr<core::Type> &tpe) {
        auto lit = std::make_unique<ast::Literal>(loc, tpe);
        return lit;
    }

    static std::unique_ptr<Expression> Return(core::Loc loc, std::unique_ptr<Expression> expr) {
        return std::make_unique<ast::Return>(loc, move(expr));
    }

    static std::unique_ptr<Expression> Next(core::Loc loc, std::unique_ptr<Expression> expr) {
        return std::make_unique<ast::Next>(loc, move(expr));
    }

    static std::unique_ptr<Expression> Yield(core::Loc loc, Send::ARGS_store args) {
        return std::make_unique<ast::Yield>(loc, move(args));
    }

    static std::unique_ptr<Expression> Break(core::Loc loc, std::unique_ptr<Expression> expr) {
        return std::make_unique<ast::Break>(loc, move(expr));
    }

    static std::unique_ptr<Expression> Send0(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun) {
        Send::ARGS_store nargs;
        return Send(loc, move(recv), fun, move(nargs));
    }

    static std::unique_ptr<Expression> Send1(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                             std::unique_ptr<Expression> arg1) {
        Send::ARGS_store nargs;
        nargs.emplace_back(move(arg1));
        return Send(loc, move(recv), fun, move(nargs));
    }

    static std::unique_ptr<Expression> Send2(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                             std::unique_ptr<Expression> arg1, std::unique_ptr<Expression> arg2) {
        Send::ARGS_store nargs;
        nargs.emplace_back(move(arg1));
        nargs.emplace_back(move(arg2));
        return Send(loc, move(recv), fun, move(nargs));
    }

    static std::unique_ptr<Expression> Send3(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                             std::unique_ptr<Expression> arg1, std::unique_ptr<Expression> arg2,
                                             std::unique_ptr<Expression> arg3) {
        Send::ARGS_store nargs;
        nargs.emplace_back(move(arg1));
        nargs.emplace_back(move(arg2));
        nargs.emplace_back(move(arg3));
        return Send(loc, move(recv), fun, move(nargs));
    }

    static std::unique_ptr<Expression> Nil(core::Loc loc) {
        return std::make_unique<ast::Literal>(loc, core::Types::nilClass());
    }

    static std::unique_ptr<Expression> Constant(core::Loc loc, core::SymbolRef symbol) {
        ENFORCE(symbol.exists());
        return std::make_unique<ast::ConstantLit>(loc, symbol, nullptr, nullptr);
    }

    static std::unique_ptr<Reference> Local(core::Loc loc, core::NameRef name) {
        return std::make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Local, name);
    }

    static std::unique_ptr<Reference> KeywordArg(core::Loc loc, std::unique_ptr<Reference> inner) {
        return std::make_unique<ast::KeywordArg>(loc, move(inner));
    }

    static std::unique_ptr<Reference> RestArg(core::Loc loc, std::unique_ptr<Reference> inner) {
        return std::make_unique<ast::RestArg>(loc, move(inner));
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
            s->args.emplace_back(move(rhs));
            return lhs;
        }

        return std::make_unique<ast::Assign>(loc, move(lhs), move(rhs));
    }

    static std::unique_ptr<Expression> Assign(core::Loc loc, core::NameRef name, std::unique_ptr<Expression> rhs) {
        return Assign(loc, Local(loc, name), move(rhs));
    }

    static std::unique_ptr<Expression> If(core::Loc loc, std::unique_ptr<Expression> cond,
                                          std::unique_ptr<Expression> thenp, std::unique_ptr<Expression> elsep) {
        return std::make_unique<ast::If>(loc, move(cond), move(thenp), move(elsep));
    }

    static std::unique_ptr<Expression> While(core::Loc loc, std::unique_ptr<Expression> cond,
                                             std::unique_ptr<Expression> body) {
        return std::make_unique<ast::While>(loc, move(cond), move(body));
    }

    static std::unique_ptr<Expression> Self(core::Loc loc) {
        return std::make_unique<ast::Self>(loc, core::Symbols::todo());
    }

    static std::unique_ptr<Expression> InsSeq(core::Loc loc, InsSeq::STATS_store stats,
                                              std::unique_ptr<Expression> expr) {
        if (!stats.empty()) {
            return std::make_unique<ast::InsSeq>(loc, move(stats), move(expr));
        }
        return expr;
    }

    static std::unique_ptr<Expression> Splat(core::Loc loc, std::unique_ptr<Expression> arg) {
        auto to_a = Send0(loc, move(arg), core::Names::to_a());
        return Send1(loc, Constant(loc, core::Symbols::Magic()), core::Names::splat(), move(to_a));
    }

    static std::unique_ptr<Expression> InsSeq1(core::Loc loc, std::unique_ptr<Expression> stat,
                                               std::unique_ptr<Expression> expr) {
        InsSeq::STATS_store stats;
        stats.emplace_back(move(stat));
        return InsSeq(loc, move(stats), move(expr));
    }

    static std::unique_ptr<Expression> True(core::Loc loc) {
        return std::make_unique<ast::Literal>(loc, core::Types::trueClass());
    }

    static std::unique_ptr<Expression> False(core::Loc loc) {
        return std::make_unique<ast::Literal>(loc, core::Types::falseClass());
    }

    static std::unique_ptr<UnresolvedConstantLit> UnresolvedConstant(core::Loc loc, std::unique_ptr<Expression> scope,
                                                                     core::NameRef name) {
        return std::make_unique<ast::UnresolvedConstantLit>(loc, move(scope), name);
    }

    static std::unique_ptr<Expression> Int(core::Loc loc, int64_t val) {
        return std::make_unique<ast::Literal>(loc, std::make_shared<core::LiteralType>(val));
    }

    static std::unique_ptr<Expression> Float(core::Loc loc, double val) {
        return std::make_unique<ast::Literal>(loc, std::make_shared<core::LiteralType>(val));
    }

    static std::unique_ptr<Expression> Symbol(core::Loc loc, core::NameRef name) {
        return std::make_unique<ast::Literal>(loc, std::make_shared<core::LiteralType>(core::Symbols::Symbol(), name));
    }

    static std::unique_ptr<Expression> String(core::Loc loc, core::NameRef value) {
        return std::make_unique<ast::Literal>(loc, std::make_shared<core::LiteralType>(core::Symbols::String(), value));
    }

    static std::unique_ptr<MethodDef> Method(core::Loc loc, core::Loc declLoc, core::NameRef name,
                                             MethodDef::ARGS_store args, std::unique_ptr<Expression> rhs,
                                             u4 flags = 0) {
        return std::make_unique<MethodDef>(loc, declLoc, core::Symbols::todo(), name, move(args), move(rhs), flags);
    }

    static std::unique_ptr<Expression> Method0(core::Loc loc, core::Loc declLoc, core::NameRef name,
                                               std::unique_ptr<Expression> rhs, u4 flags = 0) {
        return Method(loc, declLoc, name, {}, move(rhs), flags);
    }

    static std::unique_ptr<Expression> Method1(core::Loc loc, core::Loc declLoc, core::NameRef name,
                                               std::unique_ptr<Expression> arg0, std::unique_ptr<Expression> rhs,
                                               u4 flags = 0) {
        MethodDef::ARGS_store args;
        args.emplace_back(move(arg0));
        return Method(loc, declLoc, name, move(args), move(rhs), flags);
    }

    static std::unique_ptr<ClassDef> Class(core::Loc loc, core::Loc declLoc, std::unique_ptr<Expression> name,
                                           ClassDef::ANCESTORS_store ancestors, ClassDef::RHS_store rhs,
                                           ClassDefKind kind) {
        return std::make_unique<ClassDef>(loc, declLoc, core::Symbols::todo(), move(name), move(ancestors), move(rhs),
                                          kind);
    }

    static std::unique_ptr<Expression> Array(core::Loc loc, Array::ENTRY_store entries) {
        return std::make_unique<ast::Array>(loc, move(entries));
    }

    static std::unique_ptr<Expression> Hash(core::Loc loc, Hash::ENTRY_store keys, Hash::ENTRY_store values) {
        return std::make_unique<ast::Hash>(loc, move(keys), move(values));
    }

    static std::unique_ptr<Expression> Hash0(core::Loc loc) {
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        return Hash(loc, move(keys), move(values));
    }

    static std::unique_ptr<Expression> Hash1(core::Loc loc, std::unique_ptr<Expression> key,
                                             std::unique_ptr<Expression> value) {
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        keys.emplace_back(move(key));
        values.emplace_back(move(value));
        return Hash(loc, move(keys), move(values));
    }

    static std::unique_ptr<Expression> Sig(core::Loc loc, std::unique_ptr<Expression> hash,
                                           std::unique_ptr<Expression> ret) {
        auto params = Send1(loc, Self(loc), core::Names::params(), move(hash));
        auto returns = Send1(loc, move(params), core::Names::returns(), move(ret));
        auto sig = Send0(loc, Constant(loc, core::Symbols::Sorbet()), core::Names::sig());
        auto sigSend = ast::cast_tree<ast::Send>(sig.get());
        sigSend->block = Block0(loc, move(returns));
        return sig;
    }

    static std::unique_ptr<Expression> Sig0(core::Loc loc, std::unique_ptr<Expression> ret) {
        auto returns = Send1(loc, Self(loc), core::Names::returns(), move(ret));
        auto sig = Send0(loc, Constant(loc, core::Symbols::Sorbet()), core::Names::sig());
        auto sigSend = ast::cast_tree<ast::Send>(sig.get());
        sigSend->block = Block0(loc, move(returns));
        return sig;
    }

    static std::unique_ptr<Expression> Sig1(core::Loc loc, std::unique_ptr<Expression> key,
                                            std::unique_ptr<Expression> value, std::unique_ptr<Expression> ret) {
        return Sig(loc, Hash1(loc, move(key), move(value)), move(ret));
    }

    static std::unique_ptr<Expression> Cast(core::Loc loc, std::unique_ptr<Expression> type) {
        if (auto *send = cast_tree<ast::Send>(type.get())) {
            if (send->fun == core::Names::untyped()) {
                return Unsafe(loc, Nil(loc));
            }
        }
        return Send2(loc, Constant(loc, core::Symbols::T()), core::Names::cast(), Unsafe(loc, Nil(loc)), move(type));
    }

    static std::unique_ptr<Expression> T(core::Loc loc) {
        return Constant(loc, core::Symbols::T());
    }

    static std::unique_ptr<Expression> Let(core::Loc loc, std::unique_ptr<Expression> value,
                                           std::unique_ptr<Expression> type) {
        return Send2(loc, T(loc), core::Names::let(), move(value), move(type));
    }

    static std::unique_ptr<Expression> Unsafe(core::Loc loc, std::unique_ptr<Expression> inner) {
        return Send1(loc, T(loc), core::Names::unsafe(), move(inner));
    }

    static std::unique_ptr<Expression> Untyped(core::Loc loc) {
        return Send0(loc, T(loc), core::Names::untyped());
    }

    static std::unique_ptr<Expression> Nilable(core::Loc loc, std::unique_ptr<Expression> arg) {
        return Send1(loc, T(loc), core::Names::nilable(), move(arg));
    }

    static std::unique_ptr<Expression> KeepForIDE(std::unique_ptr<Expression> arg) {
        auto loc = core::Loc::none(arg->loc.file());
        return Send1(loc, Constant(loc, core::Symbols::RubyTyper()), core::Names::keepForIde(), move(arg));
    }

    static std::unique_ptr<Expression> KeepForTypechecking(std::unique_ptr<Expression> arg) {
        auto loc = core::Loc::none(arg->loc.file());
        return Send1(loc, Constant(loc, core::Symbols::RubyTyper()), core::Names::keepForTypechecking(), move(arg));
    }
};

} // namespace sorbet::ast

#endif // SORBET_TREES_H
