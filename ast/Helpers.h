#ifndef SORBET_AST_HELPERSS_H
#define SORBET_AST_HELPERSS_H

#include "ast/ast.h"
#include "common/typecase.h"
#include "core/Names.h"

namespace sorbet::ast {

class MK {
public:
    static TreePtr EmptyTree() {
        return make_tree<ast::EmptyTree>();
    }

    static TreePtr Block(core::LocOffsets loc, TreePtr body, MethodDef::ARGS_store args) {
        return make_tree<ast::Block>(loc, std::move(args), std::move(body));
    }

    static TreePtr Block0(core::LocOffsets loc, TreePtr body) {
        MethodDef::ARGS_store args;
        return Block(loc, std::move(body), std::move(args));
    }

    static TreePtr Block1(core::LocOffsets loc, TreePtr body, TreePtr arg1) {
        MethodDef::ARGS_store args;
        args.emplace_back(std::move(arg1));
        return Block(loc, std::move(body), std::move(args));
    }

    static TreePtr Send(core::LocOffsets loc, TreePtr recv, core::NameRef fun, Send::ARGS_store args,
                        Send::Flags flags = {}, TreePtr blk = nullptr) {
        auto send = make_tree<ast::Send>(loc, std::move(recv), fun, std::move(args), std::move(blk), flags);
        return send;
    }

    static TreePtr Send0(core::LocOffsets loc, TreePtr recv, core::NameRef fun) {
        Send::ARGS_store nargs;
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static TreePtr Send0Block(core::LocOffsets loc, TreePtr recv, core::NameRef fun, TreePtr blk) {
        Send::ARGS_store nargs;
        return Send(loc, std::move(recv), fun, std::move(nargs), {}, std::move(blk));
    }

    static TreePtr Send1(core::LocOffsets loc, TreePtr recv, core::NameRef fun, TreePtr arg1) {
        Send::ARGS_store nargs;
        nargs.emplace_back(std::move(arg1));
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static TreePtr Send2(core::LocOffsets loc, TreePtr recv, core::NameRef fun, TreePtr arg1, TreePtr arg2) {
        Send::ARGS_store nargs;
        nargs.emplace_back(std::move(arg1));
        nargs.emplace_back(std::move(arg2));
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static TreePtr Send3(core::LocOffsets loc, TreePtr recv, core::NameRef fun, TreePtr arg1, TreePtr arg2,
                         TreePtr arg3) {
        Send::ARGS_store nargs;
        nargs.emplace_back(std::move(arg1));
        nargs.emplace_back(std::move(arg2));
        nargs.emplace_back(std::move(arg3));
        return Send(loc, std::move(recv), fun, std::move(nargs));
    }

    static TreePtr Literal(core::LocOffsets loc, const core::TypePtr &tpe) {
        return make_tree<ast::Literal>(loc, tpe);
    }

    static TreePtr Return(core::LocOffsets loc, TreePtr expr) {
        return make_tree<ast::Return>(loc, std::move(expr));
    }

    static TreePtr Next(core::LocOffsets loc, TreePtr expr) {
        return make_tree<ast::Next>(loc, std::move(expr));
    }

    static TreePtr Break(core::LocOffsets loc, TreePtr expr) {
        return make_tree<ast::Break>(loc, std::move(expr));
    }

    static TreePtr Nil(core::LocOffsets loc) {
        return make_tree<ast::Literal>(loc, core::Types::nilClass());
    }

    static TreePtr Constant(core::LocOffsets loc, core::SymbolRef symbol) {
        ENFORCE(symbol.exists());
        return make_tree<ConstantLit>(loc, symbol, nullptr);
    }

    static TreePtr Local(core::LocOffsets loc, core::NameRef name) {
        return make_tree<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Local, name);
    }

    static TreePtr OptionalArg(core::LocOffsets loc, TreePtr inner, TreePtr default_) {
        return make_tree<ast::OptionalArg>(loc, std::move(inner), std::move(default_));
    }

    static TreePtr KeywordArg(core::LocOffsets loc, TreePtr inner) {
        return make_tree<ast::KeywordArg>(loc, std::move(inner));
    }

    static TreePtr RestArg(core::LocOffsets loc, TreePtr inner) {
        return make_tree<ast::RestArg>(loc, std::move(inner));
    }

    static TreePtr BlockArg(core::LocOffsets loc, TreePtr inner) {
        return make_tree<ast::BlockArg>(loc, std::move(inner));
    }

    static TreePtr ShadowArg(core::LocOffsets loc, TreePtr inner) {
        return make_tree<ast::ShadowArg>(loc, std::move(inner));
    }

    static TreePtr Instance(core::LocOffsets loc, core::NameRef name) {
        return make_tree<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Instance, name);
    }

    static TreePtr cpRef(TreePtr &name) {
        if (auto *nm = cast_tree<UnresolvedIdent>(name)) {
            return make_tree<UnresolvedIdent>(nm->loc, nm->kind, nm->name);
        } else if (auto *nm = cast_tree<ast::Local>(name)) {
            return make_tree<ast::Local>(nm->loc, nm->localVariable);
        }
        Exception::notImplemented();
    }

    static TreePtr Assign(core::LocOffsets loc, TreePtr lhs, TreePtr rhs) {
        if (auto *s = cast_tree<ast::Send>(lhs)) {
            // the LHS might be a send of the form x.y=(), in which case we add the RHS to the arguments list and get
            // x.y=(rhs)
            s->args.emplace_back(std::move(rhs));
            return lhs;
        } else if (auto *seq = cast_tree<ast::InsSeq>(lhs)) {
            // the LHS might be a sequence, which means that it's the result of a safe navigation operator, like
            //   { $t = x; if $t == nil then nil else $t.y=() }
            // in which case we just need to dril down into the else-case of the condition and add the rhs to the send
            //   { $t = x; if $t == nil then nil else $t.y=(rhs)
            if (auto *cond = cast_tree<ast::If>(seq->expr)) {
                if (auto *s = cast_tree<ast::Send>(cond->elsep)) {
                    s->args.emplace_back(std::move(rhs));
                    return lhs;
                }
            }
        }

        // otherwise, just assign to it!
        return make_tree<ast::Assign>(loc, std::move(lhs), std::move(rhs));
    }

    static TreePtr Assign(core::LocOffsets loc, core::NameRef name, TreePtr rhs) {
        return Assign(loc, Local(loc, name), std::move(rhs));
    }

    static TreePtr If(core::LocOffsets loc, TreePtr cond, TreePtr thenp, TreePtr elsep) {
        return make_tree<ast::If>(loc, std::move(cond), std::move(thenp), std::move(elsep));
    }

    static TreePtr While(core::LocOffsets loc, TreePtr cond, TreePtr body) {
        return make_tree<ast::While>(loc, std::move(cond), std::move(body));
    }

    static TreePtr Self(core::LocOffsets loc) {
        return make_tree<ast::Local>(loc, core::LocalVariable::selfVariable());
    }

    static TreePtr InsSeq(core::LocOffsets loc, InsSeq::STATS_store stats, TreePtr expr) {
        if (!stats.empty()) {
            return make_tree<ast::InsSeq>(loc, std::move(stats), std::move(expr));
        }
        return expr;
    }

    static TreePtr Splat(core::LocOffsets loc, TreePtr arg) {
        auto to_a = Send0(loc, std::move(arg), core::Names::toA());
        return Send1(loc, Constant(loc, core::Symbols::Magic()), core::Names::splat(), std::move(to_a));
    }

    static TreePtr CallWithSplat(core::LocOffsets loc, TreePtr recv, core::NameRef name, TreePtr args) {
        return Send3(loc, Constant(loc, core::Symbols::Magic()), core::Names::callWithSplat(), std::move(recv),
                     MK::Symbol(loc, name), std::move(args));
    }

    static TreePtr InsSeq1(core::LocOffsets loc, TreePtr stat, TreePtr expr) {
        InsSeq::STATS_store stats;
        stats.emplace_back(std::move(stat));
        return InsSeq(loc, std::move(stats), std::move(expr));
    }

    static TreePtr True(core::LocOffsets loc) {
        return make_tree<ast::Literal>(loc, core::Types::trueClass());
    }

    static TreePtr False(core::LocOffsets loc) {
        return make_tree<ast::Literal>(loc, core::Types::falseClass());
    }

    static TreePtr UnresolvedConstant(core::LocOffsets loc, TreePtr scope, core::NameRef name) {
        return make_tree<UnresolvedConstantLit>(loc, std::move(scope), name);
    }

    static TreePtr Int(core::LocOffsets loc, int64_t val) {
        return make_tree<ast::Literal>(loc, core::make_type<core::LiteralType>(val));
    }

    static TreePtr Float(core::LocOffsets loc, double val) {
        return make_tree<ast::Literal>(loc, core::make_type<core::LiteralType>(val));
    }

    static TreePtr Symbol(core::LocOffsets loc, core::NameRef name) {
        return make_tree<ast::Literal>(loc, core::make_type<core::LiteralType>(core::Symbols::Symbol(), name));
    }

    static TreePtr String(core::LocOffsets loc, core::NameRef value) {
        return make_tree<ast::Literal>(loc, core::make_type<core::LiteralType>(core::Symbols::String(), value));
    }

    static TreePtr Method(core::LocOffsets loc, core::Loc declLoc, core::NameRef name, MethodDef::ARGS_store args,
                          TreePtr rhs) {
        if (args.empty() || (!isa_tree<ast::Local>(args.back()) && !isa_tree<ast::BlockArg>(args.back()))) {
            auto blkLoc = core::LocOffsets::none();
            args.emplace_back(make_tree<ast::BlockArg>(blkLoc, MK::Local(blkLoc, core::Names::blkArg())));
        }
        MethodDef::Flags flags;
        return make_tree<MethodDef>(loc, declLoc, core::Symbols::todo(), name, std::move(args), std::move(rhs), flags);
    }

    static TreePtr SyntheticMethod(core::LocOffsets loc, core::Loc declLoc, core::NameRef name,
                                   MethodDef::ARGS_store args, TreePtr rhs) {
        auto mdef = Method(loc, declLoc, name, std::move(args), std::move(rhs));
        cast_tree<MethodDef>(mdef)->flags.isRewriterSynthesized = true;
        return mdef;
    }

    static TreePtr SyntheticMethod0(core::LocOffsets loc, core::Loc declLoc, core::NameRef name, TreePtr rhs) {
        MethodDef::ARGS_store args;
        return SyntheticMethod(loc, declLoc, name, std::move(args), std::move(rhs));
    }

    static TreePtr SyntheticMethod1(core::LocOffsets loc, core::Loc declLoc, core::NameRef name, TreePtr arg0,
                                    TreePtr rhs) {
        MethodDef::ARGS_store args;
        args.emplace_back(std::move(arg0));
        return SyntheticMethod(loc, declLoc, name, std::move(args), std::move(rhs));
    }

    static TreePtr ClassOrModule(core::LocOffsets loc, core::Loc declLoc, TreePtr name,
                                 ClassDef::ANCESTORS_store ancestors, ClassDef::RHS_store rhs, ClassDef::Kind kind) {
        return make_tree<ClassDef>(loc, declLoc, core::Symbols::todo(), std::move(name), std::move(ancestors),
                                   std::move(rhs), kind);
    }

    static TreePtr Class(core::LocOffsets loc, core::Loc declLoc, TreePtr name, ClassDef::ANCESTORS_store ancestors,
                         ClassDef::RHS_store rhs) {
        return MK::ClassOrModule(loc, declLoc, std::move(name), std::move(ancestors), std::move(rhs),
                                 ClassDef::Kind::Class);
    }

    static TreePtr Module(core::LocOffsets loc, core::Loc declLoc, TreePtr name, ClassDef::ANCESTORS_store ancestors,
                          ClassDef::RHS_store rhs) {
        return MK::ClassOrModule(loc, declLoc, std::move(name), std::move(ancestors), std::move(rhs),
                                 ClassDef::Kind::Module);
    }

    static TreePtr Array(core::LocOffsets loc, Array::ENTRY_store entries) {
        return make_tree<ast::Array>(loc, std::move(entries));
    }

    static TreePtr Hash(core::LocOffsets loc, Hash::ENTRY_store keys, Hash::ENTRY_store values) {
        return make_tree<ast::Hash>(loc, std::move(keys), std::move(values));
    }

    static TreePtr Hash0(core::LocOffsets loc) {
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        return Hash(loc, std::move(keys), std::move(values));
    }

    static TreePtr Hash1(core::LocOffsets loc, TreePtr key, TreePtr value) {
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        keys.emplace_back(std::move(key));
        values.emplace_back(std::move(value));
        return Hash(loc, std::move(keys), std::move(values));
    }

    static TreePtr Sig(core::LocOffsets loc, TreePtr hash, TreePtr ret) {
        auto params = Send1(loc, Self(loc), core::Names::params(), std::move(hash));
        auto returns = Send1(loc, std::move(params), core::Names::returns(), std::move(ret));
        auto sig = Send1(loc, Constant(loc, core::Symbols::Sorbet_Private_Static()), core::Names::sig(),
                         Constant(loc, core::Symbols::T_Sig_WithoutRuntime()));
        auto sigSend = ast::cast_tree<ast::Send>(sig);
        sigSend->block = Block0(loc, std::move(returns));
        sigSend->flags.isRewriterSynthesized = true;
        return sig;
    }

    static TreePtr SigVoid(core::LocOffsets loc, TreePtr hash) {
        auto params = Send1(loc, Self(loc), core::Names::params(), std::move(hash));
        auto void_ = Send0(loc, std::move(params), core::Names::void_());
        auto sig = Send1(loc, Constant(loc, core::Symbols::Sorbet_Private_Static()), core::Names::sig(),
                         Constant(loc, core::Symbols::T_Sig_WithoutRuntime()));
        auto sigSend = ast::cast_tree<ast::Send>(sig);
        sigSend->block = Block0(loc, std::move(void_));
        sigSend->flags.isRewriterSynthesized = true;
        return sig;
    }

    static TreePtr Sig0(core::LocOffsets loc, TreePtr ret) {
        auto returns = Send1(loc, Self(loc), core::Names::returns(), std::move(ret));
        auto sig = Send1(loc, Constant(loc, core::Symbols::Sorbet_Private_Static()), core::Names::sig(),
                         Constant(loc, core::Symbols::T_Sig_WithoutRuntime()));
        auto sigSend = ast::cast_tree<ast::Send>(sig);
        sigSend->block = Block0(loc, std::move(returns));
        sigSend->flags.isRewriterSynthesized = true;
        return sig;
    }

    static TreePtr Sig1(core::LocOffsets loc, TreePtr key, TreePtr value, TreePtr ret) {
        return Sig(loc, Hash1(loc, std::move(key), std::move(value)), std::move(ret));
    }

    static TreePtr T(core::LocOffsets loc) {
        return Constant(loc, core::Symbols::T());
    }

    static TreePtr Let(core::LocOffsets loc, TreePtr value, TreePtr type) {
        return Send2(loc, T(loc), core::Names::let(), std::move(value), std::move(type));
    }

    static TreePtr AssertType(core::LocOffsets loc, TreePtr value, TreePtr type) {
        return Send2(loc, T(loc), core::Names::assertType(), std::move(value), std::move(type));
    }

    static TreePtr Unsafe(core::LocOffsets loc, TreePtr inner) {
        return Send1(loc, T(loc), core::Names::unsafe(), std::move(inner));
    }

    static TreePtr Untyped(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::untyped());
    }

    static TreePtr Nilable(core::LocOffsets loc, TreePtr arg) {
        return Send1(loc, T(loc), core::Names::nilable(), std::move(arg));
    }

    static TreePtr KeepForIDE(TreePtr arg) {
        auto loc = core::LocOffsets::none();
        return Send1(loc, Constant(loc, core::Symbols::Sorbet_Private_Static()), core::Names::keepForIde(),
                     std::move(arg));
    }

    static TreePtr KeepForTypechecking(TreePtr arg) {
        auto loc = core::LocOffsets::none();
        return Send1(loc, Constant(loc, core::Symbols::Sorbet_Private_Static()), core::Names::keepForTypechecking(),
                     std::move(arg));
    }

    static TreePtr ZSuper(core::LocOffsets loc) {
        return Send1(loc, Self(loc), core::Names::super(), make_tree<ast::ZSuperArgs>(loc));
    }

    static TreePtr SelfNew(core::LocOffsets loc, ast::Send::ARGS_store args, Send::Flags flags = {},
                           TreePtr block = nullptr) {
        auto magic = Constant(loc, core::Symbols::Magic());
        return Send(loc, std::move(magic), core::Names::selfNew(), std::move(args), flags, std::move(block));
    }

    static TreePtr DefineTopClassOrModule(core::LocOffsets loc, core::SymbolRef klass) {
        auto magic = Constant(loc, core::Symbols::Magic());
        Send::ARGS_store args;
        args.emplace_back(Constant(loc, klass));
        Send::Flags flags;
        flags.isRewriterSynthesized = true;
        return Send(loc, std::move(magic), core::Names::defineTopClassOrModule(), std::move(args), flags);
    }

    static TreePtr RaiseUnimplemented(core::LocOffsets loc) {
        auto kernel = Constant(loc, core::Symbols::Kernel());
        auto msg = String(loc, core::Names::rewriterRaiseUnimplemented());
        // T.unsafe so that Sorbet doesn't know this unconditionally raises (avoids introducing dead code errors)
        auto ret = Send1(loc, Unsafe(loc, std::move(kernel)), core::Names::raise(), std::move(msg));
        cast_tree<ast::Send>(ret)->flags.isRewriterSynthesized = true;
        return ret;
    }

    static bool isRootScope(ast::TreePtr &scope) {
        if (ast::isa_tree<ast::EmptyTree>(scope)) {
            return true;
        }
        auto root = ast::cast_tree<ast::ConstantLit>(scope);
        return root != nullptr && root->symbol == core::Symbols::root();
    }

    static bool isMagicClass(TreePtr &expr) {
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

        return isMagicClass(send->recv);
    }

    static ast::Local const *arg2Local(const ast::TreePtr &arg) {
        auto *cursor = &arg;
        while (true) {
            if (auto *local = ast::cast_tree_const<ast::Local>(*cursor)) {
                // Buried deep within every argument is a Local
                return local;
            }

            // Recurse into structure to find the Local
            typecase(
                cursor->get(), [&](class RestArg *rest) { cursor = &rest->expr; },
                [&](class KeywordArg *kw) { cursor = &kw->expr; }, [&](class OptionalArg *opt) { cursor = &opt->expr; },
                [&](class BlockArg *blk) { cursor = &blk->expr; },
                [&](class ShadowArg *shadow) { cursor = &shadow->expr; },
                // ENFORCES are last so that we don't pay the price of casting in the fast path.
                [&](UnresolvedIdent *opt) { ENFORCE(false, "Namer should have created a Local for this arg."); },
                [&](Expression *expr) { ENFORCE(false, "Unexpected node type in argument position."); });
        }
    }
};

class BehaviorHelpers final {
public:
    // Recursively check if all children of an expression are EmptyTree's or InsSeq's that only contain EmptyTree's
    static bool checkEmptyDeep(const TreePtr &);

    // Does a class/module definition define "behavior"? A class definition that only serves as a
    // namespace for inner-definitions is not considered to have behavior.
    //
    // module A
    //   CONST = true <-- Behavior on A::CONST (not A)
    //   module B
    //     def m; end <-- Behavior in A::B
    //   end
    // end
    static bool checkClassDefinesBehavior(const TreePtr &);
    static bool checkClassDefinesBehavior(const ast::ClassDef &);
};

} // namespace sorbet::ast

#endif // SORBET_TREES_H
