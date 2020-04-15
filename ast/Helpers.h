#ifndef SORBET_AST_HELPERSS_H
#define SORBET_AST_HELPERSS_H

#include "ast/ast.h"
#include "common/typecase.h"
#include "core/Names.h"

namespace sorbet::ast {

class MK {
public:
    static std::unique_ptr<ast::EmptyTree> EmptyTree() {
        return std::make_unique<ast::EmptyTree>();
    }

    static std::unique_ptr<Block> Block(core::LocOffsets loc, std::unique_ptr<Expression> body,
                                        MethodDef::ARGS_store args) {
        auto blk = std::make_unique<ast::Block>(loc, std::move(args), std::move(body));
        return blk;
    }

    static std::unique_ptr<ast::Block> Block0(core::LocOffsets loc, std::unique_ptr<Expression> body) {
        MethodDef::ARGS_store args;
        return Block(loc, std::move(body), std::move(args));
    }

    static std::unique_ptr<ast::Block> Block1(core::LocOffsets loc, std::unique_ptr<Expression> body,
                                              std::unique_ptr<Expression> arg1) {
        MethodDef::ARGS_store args;
        args.emplace_back(move(arg1));
        return Block(loc, std::move(body), std::move(args));
    }

    static std::unique_ptr<Send> Send(core::LocOffsets loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                      Send::ARGS_store args, Send::Flags flags = {},
                                      std::unique_ptr<ast::Block> blk = nullptr) {
        auto send = std::make_unique<ast::Send>(loc, std::move(recv), fun, std::move(args), std::move(blk), flags);
        return send;
    }

    static std::unique_ptr<ast::Send> Send0(core::LocOffsets loc, std::unique_ptr<Expression> recv, core::NameRef fun) {
        Send::ARGS_store nargs;
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static std::unique_ptr<ast::Send> Send0Block(core::LocOffsets loc, std::unique_ptr<Expression> recv,
                                                 core::NameRef fun, std::unique_ptr<ast::Block> blk) {
        Send::ARGS_store nargs;
        return Send(loc, std::move(recv), fun, std::move(nargs), {}, std::move(blk));
    }

    static std::unique_ptr<ast::Send> Send1(core::LocOffsets loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                            std::unique_ptr<Expression> arg1) {
        Send::ARGS_store nargs;
        nargs.emplace_back(std::move(arg1));
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static std::unique_ptr<ast::Send> Send2(core::LocOffsets loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                            std::unique_ptr<Expression> arg1, std::unique_ptr<Expression> arg2) {
        Send::ARGS_store nargs;
        nargs.emplace_back(std::move(arg1));
        nargs.emplace_back(std::move(arg2));
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static std::unique_ptr<ast::Send> Send3(core::LocOffsets loc, std::unique_ptr<Expression> recv, core::NameRef fun,
                                            std::unique_ptr<Expression> arg1, std::unique_ptr<Expression> arg2,
                                            std::unique_ptr<Expression> arg3) {
        Send::ARGS_store nargs;
        nargs.emplace_back(std::move(arg1));
        nargs.emplace_back(std::move(arg2));
        nargs.emplace_back(std::move(arg3));
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static std::unique_ptr<Literal> Literal(core::LocOffsets loc, const core::TypePtr &tpe) {
        return std::make_unique<ast::Literal>(loc, tpe);
    }

    static std::unique_ptr<ast::Return> Return(core::LocOffsets loc, std::unique_ptr<Expression> expr) {
        return std::make_unique<ast::Return>(loc, std::move(expr));
    }

    static std::unique_ptr<ast::Next> Next(core::LocOffsets loc, std::unique_ptr<Expression> expr) {
        return std::make_unique<ast::Next>(loc, std::move(expr));
    }

    static std::unique_ptr<ast::Break> Break(core::LocOffsets loc, std::unique_ptr<Expression> expr) {
        return std::make_unique<ast::Break>(loc, std::move(expr));
    }

    static std::unique_ptr<ast::Literal> Nil(core::LocOffsets loc) {
        return std::make_unique<ast::Literal>(loc, core::Types::nilClass());
    }

    static std::unique_ptr<ast::ConstantLit> Constant(core::LocOffsets loc, core::SymbolRef symbol) {
        ENFORCE(symbol.exists());
        return std::make_unique<ConstantLit>(loc, symbol, nullptr);
    }

    static std::unique_ptr<ast::UnresolvedIdent> Local(core::LocOffsets loc, core::NameRef name) {
        return std::make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Local, name);
    }

    static std::unique_ptr<ast::OptionalArg> OptionalArg(core::LocOffsets loc, std::unique_ptr<Reference> inner,
                                                         std::unique_ptr<Expression> default_) {
        return std::make_unique<ast::OptionalArg>(loc, std::move(inner), std::move(default_));
    }

    static std::unique_ptr<ast::KeywordArg> KeywordArg(core::LocOffsets loc, std::unique_ptr<Reference> inner) {
        return std::make_unique<ast::KeywordArg>(loc, std::move(inner));
    }

    static std::unique_ptr<ast::RestArg> RestArg(core::LocOffsets loc, std::unique_ptr<Reference> inner) {
        return std::make_unique<ast::RestArg>(loc, std::move(inner));
    }

    static std::unique_ptr<ast::BlockArg> BlockArg(core::LocOffsets loc, std::unique_ptr<Reference> inner) {
        return std::make_unique<ast::BlockArg>(loc, std::move(inner));
    }

    static std::unique_ptr<ast::ShadowArg> ShadowArg(core::LocOffsets loc, std::unique_ptr<Reference> inner) {
        return std::make_unique<ast::ShadowArg>(loc, std::move(inner));
    }

    static std::unique_ptr<ast::UnresolvedIdent> Instance(core::LocOffsets loc, core::NameRef name) {
        return std::make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Instance, name);
    }

    static std::unique_ptr<ast::Reference> cpRef(Reference &name) {
        if (auto *nm = cast_tree<UnresolvedIdent>(&name)) {
            return std::make_unique<UnresolvedIdent>(name.loc, nm->kind, nm->name);
        } else if (auto *nm = cast_tree<ast::Local>(&name)) {
            return std::make_unique<ast::Local>(name.loc, nm->localVariable);
        }
        Exception::notImplemented();
    }

    static std::unique_ptr<Expression> Assign(core::LocOffsets loc, std::unique_ptr<Expression> lhs,
                                              std::unique_ptr<Expression> rhs) {
        if (auto *s = cast_tree<ast::Send>(lhs.get())) {
            // the LHS might be a send of the form x.y=(), in which case we add the RHS to the arguments list and get
            // x.y=(rhs)
            s->args.emplace_back(std::move(rhs));
            return lhs;
        } else if (auto *seq = cast_tree<ast::InsSeq>(lhs.get())) {
            // the LHS might be a sequence, which means that it's the result of a safe navigation operator, like
            //   { $t = x; if $t == nil then nil else $t.y=() }
            // in which case we just need to dril down into the else-case of the condition and add the rhs to the send
            //   { $t = x; if $t == nil then nil else $t.y=(rhs)
            if (auto *cond = cast_tree<ast::If>(seq->expr.get())) {
                if (auto *s = cast_tree<ast::Send>(cond->elsep.get())) {
                    s->args.emplace_back(std::move(rhs));
                    return lhs;
                }
            }
        }

        // otherwise, just assign to it!
        return std::make_unique<ast::Assign>(loc, std::move(lhs), std::move(rhs));
    }

    static std::unique_ptr<Expression> Assign(core::LocOffsets loc, core::NameRef name,
                                              std::unique_ptr<Expression> rhs) {
        return Assign(loc, Local(loc, name), std::move(rhs));
    }

    static std::unique_ptr<ast::If> If(core::LocOffsets loc, std::unique_ptr<Expression> cond,
                                       std::unique_ptr<Expression> thenp, std::unique_ptr<Expression> elsep) {
        return std::make_unique<ast::If>(loc, std::move(cond), std::move(thenp), std::move(elsep));
    }

    static std::unique_ptr<ast::While> While(core::LocOffsets loc, std::unique_ptr<Expression> cond,
                                             std::unique_ptr<Expression> body) {
        return std::make_unique<ast::While>(loc, std::move(cond), std::move(body));
    }

    static std::unique_ptr<ast::Local> Self(core::LocOffsets loc) {
        return std::make_unique<ast::Local>(loc, core::LocalVariable::selfVariable());
    }

    static std::unique_ptr<Expression> InsSeq(core::LocOffsets loc, InsSeq::STATS_store stats,
                                              std::unique_ptr<Expression> expr) {
        if (!stats.empty()) {
            return std::make_unique<ast::InsSeq>(loc, std::move(stats), std::move(expr));
        }
        return expr;
    }

    static std::unique_ptr<ast::Send> Splat(core::LocOffsets loc, std::unique_ptr<Expression> arg) {
        auto to_a = Send0(loc, std::move(arg), core::Names::toA());
        return Send1(loc, Constant(loc, core::Symbols::Magic()), core::Names::splat(), std::move(to_a));
    }

    static std::unique_ptr<ast::Send> CallWithSplat(core::LocOffsets loc, std::unique_ptr<Expression> recv,
                                                    core::NameRef name, std::unique_ptr<Expression> args) {
        return Send3(loc, Constant(loc, core::Symbols::Magic()), core::Names::callWithSplat(), std::move(recv),
                     MK::Symbol(loc, name), std::move(args));
    }

    static std::unique_ptr<Expression> InsSeq1(core::LocOffsets loc, std::unique_ptr<Expression> stat,
                                               std::unique_ptr<Expression> expr) {
        InsSeq::STATS_store stats;
        stats.emplace_back(std::move(stat));
        return InsSeq(loc, std::move(stats), std::move(expr));
    }

    static std::unique_ptr<ast::Literal> True(core::LocOffsets loc) {
        return std::make_unique<ast::Literal>(loc, core::Types::trueClass());
    }

    static std::unique_ptr<ast::Literal> False(core::LocOffsets loc) {
        return std::make_unique<ast::Literal>(loc, core::Types::falseClass());
    }

    static std::unique_ptr<UnresolvedConstantLit>
    UnresolvedConstant(core::LocOffsets loc, std::unique_ptr<Expression> scope, core::NameRef name) {
        return std::make_unique<UnresolvedConstantLit>(loc, std::move(scope), name);
    }

    static std::unique_ptr<ast::Literal> Int(core::LocOffsets loc, int64_t val) {
        return std::make_unique<ast::Literal>(loc, core::make_type<core::LiteralType>(val));
    }

    static std::unique_ptr<ast::Literal> Float(core::LocOffsets loc, double val) {
        return std::make_unique<ast::Literal>(loc, core::make_type<core::LiteralType>(val));
    }

    static std::unique_ptr<ast::Literal> Symbol(core::LocOffsets loc, core::NameRef name) {
        return std::make_unique<ast::Literal>(loc, core::make_type<core::LiteralType>(core::Symbols::Symbol(), name));
    }

    static std::unique_ptr<ast::Literal> String(core::LocOffsets loc, core::NameRef value) {
        return std::make_unique<ast::Literal>(loc, core::make_type<core::LiteralType>(core::Symbols::String(), value));
    }

    static std::unique_ptr<MethodDef> Method(core::LocOffsets loc, core::Loc declLoc, core::NameRef name,
                                             MethodDef::ARGS_store args, std::unique_ptr<Expression> rhs) {
        if (args.empty() || (!isa_tree<ast::Local>(args.back().get()) && !isa_tree<ast::BlockArg>(args.back().get()))) {
            auto blkLoc = core::LocOffsets::none();
            args.emplace_back(std::make_unique<ast::BlockArg>(blkLoc, MK::Local(blkLoc, core::Names::blkArg())));
        }
        MethodDef::Flags flags;
        return std::make_unique<MethodDef>(loc, declLoc, core::Symbols::todo(), name, std::move(args), std::move(rhs),
                                           flags);
    }

    static std::unique_ptr<MethodDef> SyntheticMethod(core::LocOffsets loc, core::Loc declLoc, core::NameRef name,
                                                      MethodDef::ARGS_store args, std::unique_ptr<Expression> rhs) {
        auto mdef = Method(loc, declLoc, name, std::move(args), std::move(rhs));
        mdef->flags.isRewriterSynthesized = true;
        return mdef;
    }

    static std::unique_ptr<MethodDef> SyntheticMethod0(core::LocOffsets loc, core::Loc declLoc, core::NameRef name,
                                                       std::unique_ptr<Expression> rhs) {
        MethodDef::ARGS_store args;
        return SyntheticMethod(loc, declLoc, name, std::move(args), std::move(rhs));
    }

    static std::unique_ptr<MethodDef> SyntheticMethod1(core::LocOffsets loc, core::Loc declLoc, core::NameRef name,
                                                       std::unique_ptr<Expression> arg0,
                                                       std::unique_ptr<Expression> rhs) {
        MethodDef::ARGS_store args;
        args.emplace_back(std::move(arg0));
        return SyntheticMethod(loc, declLoc, name, std::move(args), std::move(rhs));
    }

    static std::unique_ptr<ClassDef> ClassOrModule(core::LocOffsets loc, core::Loc declLoc,
                                                   std::unique_ptr<Expression> name,
                                                   ClassDef::ANCESTORS_store ancestors, ClassDef::RHS_store rhs,
                                                   ClassDef::Kind kind) {
        return std::make_unique<ClassDef>(loc, declLoc, core::Symbols::todo(), std::move(name), std::move(ancestors),
                                          std::move(rhs), kind);
    }

    static std::unique_ptr<ClassDef> Class(core::LocOffsets loc, core::Loc declLoc, std::unique_ptr<Expression> name,
                                           ClassDef::ANCESTORS_store ancestors, ClassDef::RHS_store rhs) {
        return MK::ClassOrModule(loc, declLoc, std::move(name), std::move(ancestors), std::move(rhs),
                                 ClassDef::Kind::Class);
    }

    static std::unique_ptr<ClassDef> Module(core::LocOffsets loc, core::Loc declLoc, std::unique_ptr<Expression> name,
                                            ClassDef::ANCESTORS_store ancestors, ClassDef::RHS_store rhs) {
        return MK::ClassOrModule(loc, declLoc, std::move(name), std::move(ancestors), std::move(rhs),
                                 ClassDef::Kind::Module);
    }

    static std::unique_ptr<ast::Array> Array(core::LocOffsets loc, Array::ENTRY_store entries) {
        return std::make_unique<ast::Array>(loc, std::move(entries));
    }

    static std::unique_ptr<ast::Hash> Hash(core::LocOffsets loc, Hash::ENTRY_store keys, Hash::ENTRY_store values) {
        return std::make_unique<ast::Hash>(loc, std::move(keys), std::move(values));
    }

    static std::unique_ptr<ast::Hash> Hash0(core::LocOffsets loc) {
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        return Hash(loc, std::move(keys), std::move(values));
    }

    static std::unique_ptr<ast::Hash> Hash1(core::LocOffsets loc, std::unique_ptr<Expression> key,
                                            std::unique_ptr<Expression> value) {
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        keys.emplace_back(std::move(key));
        values.emplace_back(std::move(value));
        return Hash(loc, std::move(keys), std::move(values));
    }

    static std::unique_ptr<ast::Send> Sig(core::LocOffsets loc, std::unique_ptr<Expression> hash,
                                          std::unique_ptr<Expression> ret) {
        auto params = Send1(loc, Self(loc), core::Names::params(), std::move(hash));
        auto returns = Send1(loc, std::move(params), core::Names::returns(), std::move(ret));
        auto sig = Send0(loc, Constant(loc, core::Symbols::T_Sig_WithoutRuntime()), core::Names::sig());
        auto sigSend = ast::cast_tree<ast::Send>(sig.get());
        sigSend->block = Block0(loc, std::move(returns));
        sigSend->flags.isRewriterSynthesized = true;
        return sig;
    }

    static std::unique_ptr<ast::Send> SigVoid(core::LocOffsets loc, std::unique_ptr<Expression> hash) {
        auto params = Send1(loc, Self(loc), core::Names::params(), std::move(hash));
        auto void_ = Send0(loc, std::move(params), core::Names::void_());
        auto sig = Send0(loc, Constant(loc, core::Symbols::T_Sig_WithoutRuntime()), core::Names::sig());
        auto sigSend = ast::cast_tree<ast::Send>(sig.get());
        sigSend->block = Block0(loc, std::move(void_));
        sigSend->flags.isRewriterSynthesized = true;
        return sig;
    }

    static std::unique_ptr<ast::Send> Sig0(core::LocOffsets loc, std::unique_ptr<Expression> ret) {
        auto returns = Send1(loc, Self(loc), core::Names::returns(), std::move(ret));
        auto sig = Send0(loc, Constant(loc, core::Symbols::T_Sig_WithoutRuntime()), core::Names::sig());
        auto sigSend = ast::cast_tree<ast::Send>(sig.get());
        sigSend->block = Block0(loc, std::move(returns));
        sigSend->flags.isRewriterSynthesized = true;
        return sig;
    }

    static std::unique_ptr<ast::Send> Sig1(core::LocOffsets loc, std::unique_ptr<Expression> key,
                                           std::unique_ptr<Expression> value, std::unique_ptr<Expression> ret) {
        return Sig(loc, Hash1(loc, std::move(key), std::move(value)), std::move(ret));
    }

    static std::unique_ptr<ast::Send> Cast(core::LocOffsets loc, std::unique_ptr<Expression> type) {
        if (auto *send = cast_tree<ast::Send>(type.get())) {
            if (send->fun == core::Names::untyped()) {
                return Unsafe(loc, Nil(loc));
            }
        }
        return Send2(loc, T(loc), core::Names::cast(), Unsafe(loc, Nil(loc)), std::move(type));
    }

    static std::unique_ptr<ast::ConstantLit> T(core::LocOffsets loc) {
        return Constant(loc, core::Symbols::T());
    }

    static std::unique_ptr<ast::Send> Let(core::LocOffsets loc, std::unique_ptr<Expression> value,
                                          std::unique_ptr<Expression> type) {
        return Send2(loc, T(loc), core::Names::let(), std::move(value), std::move(type));
    }

    static std::unique_ptr<ast::Send> AssertType(core::LocOffsets loc, std::unique_ptr<Expression> value,
                                                 std::unique_ptr<Expression> type) {
        return Send2(loc, T(loc), core::Names::assertType(), std::move(value), std::move(type));
    }

    static std::unique_ptr<ast::Send> Unsafe(core::LocOffsets loc, std::unique_ptr<Expression> inner) {
        return Send1(loc, T(loc), core::Names::unsafe(), std::move(inner));
    }

    static std::unique_ptr<ast::Send> Untyped(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::untyped());
    }

    static std::unique_ptr<ast::Send> Nilable(core::LocOffsets loc, std::unique_ptr<Expression> arg) {
        return Send1(loc, T(loc), core::Names::nilable(), std::move(arg));
    }

    static std::unique_ptr<ast::Send> KeepForIDE(std::unique_ptr<Expression> arg) {
        auto loc = core::LocOffsets::none();
        return Send1(loc, Constant(loc, core::Symbols::Sorbet_Private_Static()), core::Names::keepForIde(),
                     std::move(arg));
    }

    static std::unique_ptr<ast::Send> KeepForTypechecking(std::unique_ptr<Expression> arg) {
        auto loc = core::LocOffsets::none();
        return Send1(loc, Constant(loc, core::Symbols::Sorbet_Private_Static()), core::Names::keepForTypechecking(),
                     std::move(arg));
    }

    static std::unique_ptr<ast::Send> SelfNew(core::LocOffsets loc, ast::Send::ARGS_store args, Send::Flags flags = {},
                                              std::unique_ptr<ast::Block> block = nullptr) {
        auto magic = Constant(loc, core::Symbols::Magic());
        return Send(loc, std::move(magic), core::Names::selfNew(), std::move(args), flags, std::move(block));
    }

    static std::unique_ptr<ast::Send> DefineTopClassOrModule(core::LocOffsets loc, core::SymbolRef klass) {
        auto magic = Constant(loc, core::Symbols::Magic());
        Send::ARGS_store args;
        args.emplace_back(Constant(loc, klass));
        Send::Flags flags;
        flags.isRewriterSynthesized = true;
        return Send(loc, std::move(magic), core::Names::defineTopClassOrModule(), std::move(args), flags);
    }

    static std::unique_ptr<ast::Send> RaiseUnimplemented(core::LocOffsets loc) {
        auto kernel = Constant(loc, core::Symbols::Kernel());
        auto msg = String(loc, core::Names::rewriterRaiseUnimplemented());
        auto ret = Send1(loc, std::move(kernel), core::Names::raise(), std::move(msg));
        ret->flags.isRewriterSynthesized = true;
        return ret;
    }

    static bool isMagicClass(ast::Expression *expr) {
        if (auto *recv = cast_tree<ConstantLit>(expr)) {
            return recv->symbol == core::Symbols::Magic();
        } else {
            return false;
        }
    }

    static bool isSelfNew(ast::Send *send) {
        if (send->fun != core::Names::selfNew()) {
            return false;
        }

        return isMagicClass(send->recv.get());
    }

    static class Local *arg2Local(Expression *arg) {
        while (true) {
            if (auto *local = cast_tree<class Local>(arg)) {
                // Buried deep within every argument is a Local
                return local;
            }

            // Recurse into structure to find the Local
            typecase(
                arg, [&](class RestArg *rest) { arg = rest->expr.get(); },
                [&](class KeywordArg *kw) { arg = kw->expr.get(); },
                [&](class OptionalArg *opt) { arg = opt->expr.get(); },
                [&](class BlockArg *blk) { arg = blk->expr.get(); },
                [&](class ShadowArg *shadow) { arg = shadow->expr.get(); },
                // ENFORCES are last so that we don't pay the price of casting in the fast path.
                [&](UnresolvedIdent *opt) { ENFORCE(false, "Namer should have created a Local for this arg."); },
                [&](Expression *expr) { ENFORCE(false, "Unexpected node type in argument position."); });
        }
    }
};

class BehaviorHelpers final {
public:
    // Recursively check if all children of an expression are EmptyTree's or InsSeq's that only contain EmptyTree's
    static bool checkEmptyDeep(const std::unique_ptr<Expression> &);

    // Does a class/module definition define "behavior"? A class definition that only serves as a
    // namespace for inner-definitions is not considered to have behavior.
    //
    // module A
    //   CONST = true <-- Behavior on A::CONST (not A)
    //   module B
    //     def m; end <-- Behavior in A::B
    //   end
    // end
    static bool checkClassDefinesBehavior(const std::unique_ptr<ClassDef> &);
};

} // namespace sorbet::ast

#endif // SORBET_TREES_H
