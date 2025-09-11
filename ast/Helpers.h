#ifndef SORBET_AST_HELPERSS_H
#define SORBET_AST_HELPERSS_H

#include "ast/ast.h"
#include "common/typecase.h"
#include "core/Names.h"

namespace sorbet::ast {

class MK {
public:
    static ExpressionPtr EmptyTree() {
        return make_expression<ast::EmptyTree>();
    }

    static ExpressionPtr Block(core::LocOffsets loc, ExpressionPtr body, MethodDef::ARGS_store args) {
        return make_expression<ast::Block>(loc, std::move(args), std::move(body));
    }

    static ExpressionPtr Block0(core::LocOffsets loc, ExpressionPtr body) {
        MethodDef::ARGS_store args;
        return Block(loc, std::move(body), std::move(args));
    }

    static ExpressionPtr Block1(core::LocOffsets loc, ExpressionPtr body, ExpressionPtr arg1) {
        MethodDef::ARGS_store args;
        args.emplace_back(std::move(arg1));
        return Block(loc, std::move(body), std::move(args));
    }

    template <typename... Args> static Send::ARGS_store SendArgs(Args &&...args) {
        Send::ARGS_store store;
        (store.emplace_back(std::forward<Args>(args)), ...);
        return store;
    }

    static ExpressionPtr Send(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun, core::LocOffsets funLoc,
                              uint16_t numPosArgs, Send::ARGS_store args, Send::Flags flags = {}) {
        auto send = make_expression<ast::Send>(loc, std::move(recv), fun, funLoc, numPosArgs, std::move(args), flags);
        return send;
    }

    static ExpressionPtr Send0(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun, core::LocOffsets funLoc) {
        Send::ARGS_store nargs;
        return Send(loc, std::move(recv), fun, funLoc, 0, std::move(nargs));
    }

    static ExpressionPtr Send0Block(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun,
                                    core::LocOffsets funLoc, ExpressionPtr blk, Send::Flags flags = {}) {
        Send::ARGS_store nargs;
        if (blk != nullptr) {
            flags.hasBlock = true;
            nargs.emplace_back(std::move(blk));
        }
        return Send(loc, std::move(recv), fun, funLoc, 0, std::move(nargs), flags);
    }

    static ExpressionPtr Send1(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun, core::LocOffsets funLoc,
                               ExpressionPtr arg1) {
        return Send(loc, std::move(recv), fun, funLoc, 1, SendArgs(std::move(arg1)));
    }

    static ExpressionPtr Send2(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun, core::LocOffsets funLoc,
                               ExpressionPtr arg1, ExpressionPtr arg2) {
        return Send(loc, std::move(recv), fun, funLoc, 2, SendArgs(std::move(arg1), std::move(arg2)));
    }

    static ExpressionPtr Send3(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun, core::LocOffsets funLoc,
                               ExpressionPtr arg1, ExpressionPtr arg2, ExpressionPtr arg3) {
        return Send(loc, std::move(recv), fun, funLoc, 3, SendArgs(std::move(arg1), std::move(arg2), std::move(arg3)));
    }

    static ExpressionPtr Send4(core::LocOffsets loc, ExpressionPtr recv, core::NameRef fun, core::LocOffsets funLoc,
                               ExpressionPtr arg1, ExpressionPtr arg2, ExpressionPtr arg3, ExpressionPtr arg4) {
        return Send(loc, std::move(recv), fun, funLoc, 3,
                    SendArgs(std::move(arg1), std::move(arg2), std::move(arg3), std::move(arg4)));
    }

    static ExpressionPtr Literal(core::LocOffsets loc, const core::TypePtr &tpe) {
        return make_expression<ast::Literal>(loc, tpe);
    }

    static ExpressionPtr Return(core::LocOffsets loc, ExpressionPtr expr) {
        return make_expression<ast::Return>(loc, std::move(expr));
    }

    static ExpressionPtr Next(core::LocOffsets loc, ExpressionPtr expr) {
        return make_expression<ast::Next>(loc, std::move(expr));
    }

    static ExpressionPtr Break(core::LocOffsets loc, ExpressionPtr expr) {
        return make_expression<ast::Break>(loc, std::move(expr));
    }

    static ExpressionPtr Nil(core::LocOffsets loc) {
        return make_expression<ast::Literal>(loc, core::Types::nilClass());
    }

    static ExpressionPtr Constant(core::LocOffsets loc, core::SymbolRef symbol) {
        ENFORCE(symbol.exists());
        return make_expression<ConstantLit>(loc, symbol);
    }

    static ExpressionPtr Local(core::LocOffsets loc, core::NameRef name) {
        return make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Local, name);
    }

    static ExpressionPtr OptionalArg(core::LocOffsets loc, ExpressionPtr inner, ExpressionPtr default_) {
        return make_expression<ast::OptionalArg>(loc, std::move(inner), std::move(default_));
    }

    static ExpressionPtr KeywordArg(core::LocOffsets loc, core::NameRef name) {
        return make_expression<ast::KeywordArg>(loc, Local(loc, name));
    }

    static ExpressionPtr KeywordArgWithDefault(core::LocOffsets loc, core::NameRef name, ExpressionPtr default_) {
        return OptionalArg(loc, KeywordArg(loc, name), std::move(default_));
    }

    static ExpressionPtr RestArg(core::LocOffsets loc, ExpressionPtr inner) {
        return make_expression<ast::RestArg>(loc, std::move(inner));
    }

    static ExpressionPtr BlockArg(core::LocOffsets loc, ExpressionPtr inner) {
        return make_expression<ast::BlockArg>(loc, std::move(inner));
    }

    static ExpressionPtr ShadowArg(core::LocOffsets loc, ExpressionPtr inner) {
        return make_expression<ast::ShadowArg>(loc, std::move(inner));
    }

    static ExpressionPtr Instance(core::LocOffsets loc, core::NameRef name) {
        return make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Instance, name);
    }

    static ExpressionPtr cpRef(ExpressionPtr &name) {
        if (auto nm = cast_tree<UnresolvedIdent>(name)) {
            return make_expression<UnresolvedIdent>(nm->loc, nm->kind, nm->name);
        } else if (auto nm = cast_tree<ast::Local>(name)) {
            return make_expression<ast::Local>(nm->loc, nm->localVariable);
        } else if (auto self = cast_tree<ast::Self>(name)) {
            return make_expression<ast::Self>(self->loc);
        }
        Exception::notImplemented();
    }

    static ExpressionPtr Assign(core::LocOffsets loc, ExpressionPtr lhs, ExpressionPtr rhs) {
        if (auto s = cast_tree<ast::Send>(lhs)) {
            // the LHS might be a send of the form x.y=(), in which case we add the RHS to the arguments list and get
            // x.y=(rhs)
            s->addPosArg(std::move(rhs));
            return lhs;
        } else if (auto seq = cast_tree<ast::InsSeq>(lhs)) {
            // the LHS might be a sequence, which means that it's the result of a safe navigation operator, like
            //   { $t = x; if $t == nil then nil else $t.y=() }
            // in which case we just need to dril down into the else-case of the condition and add the rhs to the send
            //   { $t = x; if $t == nil then nil else $t.y=(rhs)
            if (auto cond = cast_tree<ast::If>(seq->expr)) {
                if (auto s = cast_tree<ast::Send>(cond->elsep)) {
                    s->addPosArg(std::move(rhs));
                    return lhs;
                }
            }
        }

        // otherwise, just assign to it!
        return make_expression<ast::Assign>(loc, std::move(lhs), std::move(rhs));
    }

    static ExpressionPtr Assign(core::LocOffsets loc, core::NameRef name, ExpressionPtr rhs) {
        return Assign(loc, Local(loc, name), std::move(rhs));
    }

    static ExpressionPtr If(core::LocOffsets loc, ExpressionPtr cond, ExpressionPtr thenp, ExpressionPtr elsep) {
        return make_expression<ast::If>(loc, std::move(cond), std::move(thenp), std::move(elsep));
    }

    static ExpressionPtr While(core::LocOffsets loc, ExpressionPtr cond, ExpressionPtr body) {
        return make_expression<ast::While>(loc, std::move(cond), std::move(body));
    }

    static ExpressionPtr Self(core::LocOffsets loc) {
        return make_expression<ast::Self>(loc);
    }

    static ExpressionPtr InsSeq(core::LocOffsets loc, InsSeq::STATS_store stats, ExpressionPtr expr) {
        if (!stats.empty()) {
            return make_expression<ast::InsSeq>(loc, std::move(stats), std::move(expr));
        }
        return expr;
    }

    static ExpressionPtr Splat(core::LocOffsets loc, ExpressionPtr arg) {
        return Send1(loc, Magic(loc), core::Names::splat(), loc, std::move(arg));
    }

    static ExpressionPtr CallWithSplat(core::LocOffsets loc, ExpressionPtr recv, core::NameRef name,
                                       core::LocOffsets funLoc, ExpressionPtr splat) {
        return Send4(loc, Magic(loc), core::Names::callWithSplat(), loc, std::move(recv), MK::Symbol(loc, name),
                     std::move(splat), MK::Nil(loc.copyWithZeroLength()));
    }

    static ExpressionPtr InsSeq1(core::LocOffsets loc, ExpressionPtr stat, ExpressionPtr expr) {
        InsSeq::STATS_store stats;
        stats.emplace_back(std::move(stat));
        return InsSeq(loc, std::move(stats), std::move(expr));
    }

    static ExpressionPtr True(core::LocOffsets loc) {
        return make_expression<ast::Literal>(loc, core::Types::trueClass());
    }

    static ExpressionPtr False(core::LocOffsets loc) {
        return make_expression<ast::Literal>(loc, core::Types::falseClass());
    }

    static ExpressionPtr UnresolvedConstant(core::LocOffsets loc, ExpressionPtr scope, core::NameRef name) {
        return make_expression<UnresolvedConstantLit>(loc, std::move(scope), name);
    }

    static ExpressionPtr UnresolvedConstantParts(core::LocOffsets loc, absl::Span<const core::NameRef> parts) {
        auto result = EmptyTree();
        for (const auto part : parts) {
            result = UnresolvedConstant(loc, std::move(result), part);
        }
        return result;
    }

    static ExpressionPtr Int(core::LocOffsets loc, int64_t val) {
        return make_expression<ast::Literal>(loc, core::make_type<core::IntegerLiteralType>(val));
    }

    static ExpressionPtr Float(core::LocOffsets loc, double val) {
        return make_expression<ast::Literal>(loc, core::make_type<core::FloatLiteralType>(val));
    }

    static ExpressionPtr Symbol(core::LocOffsets loc, core::NameRef name) {
        return make_expression<ast::Literal>(loc,
                                             core::make_type<core::NamedLiteralType>(core::Symbols::Symbol(), name));
    }

    static ExpressionPtr String(core::LocOffsets loc, core::NameRef value) {
        return make_expression<ast::Literal>(loc,
                                             core::make_type<core::NamedLiteralType>(core::Symbols::String(), value));
    }

    static ExpressionPtr Method(core::LocOffsets loc, core::LocOffsets declLoc, core::NameRef name,
                                MethodDef::ARGS_store args, ExpressionPtr rhs,
                                MethodDef::Flags flags = MethodDef::Flags()) {
        if (args.empty() || (!isa_tree<ast::Local>(args.back()) && !isa_tree<ast::BlockArg>(args.back()))) {
            auto blkLoc = core::LocOffsets::none();
            args.emplace_back(make_expression<ast::BlockArg>(blkLoc, MK::Local(blkLoc, core::Names::blkArg())));
        }
        return make_expression<MethodDef>(loc, declLoc, core::Symbols::todoMethod(), name, std::move(args),
                                          std::move(rhs), flags);
    }

    static ExpressionPtr Method0(core::LocOffsets loc, core::LocOffsets declLoc, core::NameRef name, ExpressionPtr rhs,
                                 MethodDef::Flags flags = MethodDef::Flags()) {
        MethodDef::ARGS_store args;
        return Method(loc, declLoc, name, std::move(args), std::move(rhs), flags);
    }

    static ExpressionPtr Method1(core::LocOffsets loc, core::LocOffsets declLoc, core::NameRef name, ExpressionPtr arg0,
                                 ExpressionPtr rhs, MethodDef::Flags flags = MethodDef::Flags()) {
        MethodDef::ARGS_store args;
        args.emplace_back(std::move(arg0));
        return Method(loc, declLoc, name, std::move(args), std::move(rhs), flags);
    }

    static ExpressionPtr SyntheticMethod(core::LocOffsets loc, core::LocOffsets declLoc, core::NameRef name,
                                         MethodDef::ARGS_store args, ExpressionPtr rhs,
                                         MethodDef::Flags flags = MethodDef::Flags()) {
        flags.isRewriterSynthesized = true;
        return Method(loc, declLoc, name, std::move(args), std::move(rhs), flags);
    }

    static ExpressionPtr SyntheticMethod0(core::LocOffsets loc, core::LocOffsets declLoc, core::NameRef name,
                                          ExpressionPtr rhs, MethodDef::Flags flags = MethodDef::Flags()) {
        MethodDef::ARGS_store args;
        return SyntheticMethod(loc, declLoc, name, std::move(args), std::move(rhs), flags);
    }

    static ExpressionPtr SyntheticMethod1(core::LocOffsets loc, core::LocOffsets declLoc, core::NameRef name,
                                          ExpressionPtr arg0, ExpressionPtr rhs,
                                          MethodDef::Flags flags = MethodDef::Flags()) {
        MethodDef::ARGS_store args;
        args.emplace_back(std::move(arg0));
        return SyntheticMethod(loc, declLoc, name, std::move(args), std::move(rhs), flags);
    }

    static ExpressionPtr ClassOrModule(core::LocOffsets loc, core::LocOffsets declLoc, ExpressionPtr name,
                                       ClassDef::ANCESTORS_store ancestors, ClassDef::RHS_store rhs,
                                       ClassDef::Kind kind) {
        return make_expression<ClassDef>(loc, declLoc, core::Symbols::todo(), std::move(name), std::move(ancestors),
                                         std::move(rhs), kind);
    }

    static ExpressionPtr Class(core::LocOffsets loc, core::LocOffsets declLoc, ExpressionPtr name,
                               ClassDef::ANCESTORS_store ancestors, ClassDef::RHS_store rhs) {
        return MK::ClassOrModule(loc, declLoc, std::move(name), std::move(ancestors), std::move(rhs),
                                 ClassDef::Kind::Class);
    }

    static ExpressionPtr Module(core::LocOffsets loc, core::LocOffsets declLoc, ExpressionPtr name,
                                ClassDef::RHS_store rhs) {
        return MK::ClassOrModule(loc, declLoc, std::move(name), {}, std::move(rhs), ClassDef::Kind::Module);
    }

    static ExpressionPtr Array(core::LocOffsets loc, Array::ENTRY_store entries) {
        return make_expression<ast::Array>(loc, std::move(entries));
    }

    static ExpressionPtr Hash(core::LocOffsets loc, Hash::ENTRY_store keys, Hash::ENTRY_store values) {
        return make_expression<ast::Hash>(loc, std::move(keys), std::move(values));
    }

    static ExpressionPtr Hash0(core::LocOffsets loc) {
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        return Hash(loc, std::move(keys), std::move(values));
    }

    static ExpressionPtr Hash1(core::LocOffsets loc, ExpressionPtr key, ExpressionPtr value) {
        Hash::ENTRY_store keys;
        Hash::ENTRY_store values;
        keys.emplace_back(std::move(key));
        values.emplace_back(std::move(value));
        return Hash(loc, std::move(keys), std::move(values));
    }

private:
    static ExpressionPtr Params(core::LocOffsets loc, ExpressionPtr recv, Send::ARGS_store args) {
        ENFORCE(args.size() % 2 == 0, "Sig params must be arg name/type pairs");

        if (args.size() > 0) {
            recv = Send(loc, std::move(recv), core::Names::params(), loc, 0, std::move(args));
        }

        return recv;
    }

public:
    static ExpressionPtr Sig(core::LocOffsets loc, Send::ARGS_store args, ExpressionPtr ret,
                             ExpressionPtr recv_ = nullptr) {
        auto recv = recv_ ? std::move(recv_) : Self(loc);
        auto params = Params(loc, std::move(recv), std::move(args));
        auto returns = Send1(loc, std::move(params), core::Names::returns(), loc, std::move(ret));

        Send::Flags flags;
        flags.isRewriterSynthesized = true;
        return Send0Block(loc, Constant(loc, core::Symbols::T_Sig_WithoutRuntime()), core::Names::sig(), loc,
                          Block0(loc, std::move(returns)), flags);
    }

    static ExpressionPtr SigVoid(core::LocOffsets loc, Send::ARGS_store args, ExpressionPtr recv_ = nullptr) {
        auto recv = recv_ ? std::move(recv_) : Self(loc);
        auto params = Params(loc, std::move(recv), std::move(args));
        auto void_ = Send0(loc, std::move(params), core::Names::void_(), loc);
        Send::Flags flags;
        flags.isRewriterSynthesized = true;
        return Send0Block(loc, Constant(loc, core::Symbols::T_Sig_WithoutRuntime()), core::Names::sig(), loc,
                          Block0(loc, std::move(void_)), flags);
    }

    static ExpressionPtr Sig0(core::LocOffsets loc, ExpressionPtr ret, ExpressionPtr recv_ = nullptr) {
        auto recv = recv_ ? std::move(recv_) : Self(loc);
        auto returns = Send1(loc, std::move(recv), core::Names::returns(), loc, std::move(ret));
        Send::Flags flags;
        flags.isRewriterSynthesized = true;
        return Send0Block(loc, Constant(loc, core::Symbols::T_Sig_WithoutRuntime()), core::Names::sig(), loc,
                          Block0(loc, std::move(returns)), flags);
    }

    static ExpressionPtr Sig1(core::LocOffsets loc, ExpressionPtr key, ExpressionPtr value, ExpressionPtr ret,
                              ExpressionPtr recv_ = nullptr) {
        return Sig(loc, SendArgs(std::move(key), std::move(value)), std::move(ret), std::move(recv_));
    }

    static ExpressionPtr Override(core::LocOffsets loc, Send::ARGS_store args) {
        return Send(loc, Self(loc), core::Names::override_(), loc, 0, std::move(args));
    }

    static ExpressionPtr OverrideStrict(core::LocOffsets loc) {
        Send::ARGS_store args;
        return Override(loc, std::move(args));
    }

    static ExpressionPtr OverrideAllowIncompatible(core::LocOffsets loc, core::LocOffsets allowLoc, ExpressionPtr arg) {
        Send::ARGS_store args;
        args.push_back(Symbol(allowLoc, core::Names::allowIncompatible()));
        args.push_back(std::move(arg));
        return Override(loc, std::move(args));
    }

    static ExpressionPtr T(core::LocOffsets loc) {
        return Constant(loc, core::Symbols::T());
    }

    static ExpressionPtr SyntheticBind(core::LocOffsets loc, ExpressionPtr value, ExpressionPtr type) {
        return ast::make_expression<ast::Cast>(loc, core::Types::todo(), std::move(value), core::Names::syntheticBind(),
                                               std::move(type));
    }

    static ExpressionPtr AssumeType(core::LocOffsets loc, ExpressionPtr value, ExpressionPtr type) {
        return ast::make_expression<ast::Cast>(loc, core::Types::todo(), std::move(value), core::Names::assumeType(),
                                               std::move(type));
    }

    static ExpressionPtr ClassOf(core::LocOffsets loc, ExpressionPtr value) {
        return Send1(loc, T(loc), core::Names::classOf(), loc, std::move(value));
    }

    static ExpressionPtr All(core::LocOffsets loc, Send::ARGS_store args) {
        return Send(loc, T(loc), core::Names::all(), loc.copyWithZeroLength(), args.size(), std::move(args));
    }

    static ExpressionPtr Any(core::LocOffsets loc, Send::ARGS_store args) {
        return Send(loc, T(loc), core::Names::any(), loc.copyWithZeroLength(), args.size(), std::move(args));
    }

    static ExpressionPtr Anything(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::anything(), loc.copyWithZeroLength());
    }

    static ExpressionPtr AttachedClass(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::attachedClass(), loc.copyWithZeroLength());
    }

    static ExpressionPtr Cast(core::LocOffsets loc, ExpressionPtr value, ExpressionPtr type) {
        return ast::make_expression<ast::Cast>(loc, core::Types::todo(), std::move(value), core::Names::cast(),
                                               std::move(type));
    }

    static ExpressionPtr Let(core::LocOffsets loc, ExpressionPtr value, ExpressionPtr type) {
        return ast::make_expression<ast::Cast>(loc, core::Types::todo(), std::move(value), core::Names::let(),
                                               std::move(type));
    }

    static ExpressionPtr AssertType(core::LocOffsets loc, ExpressionPtr value, ExpressionPtr type) {
        return ast::make_expression<ast::Cast>(loc, core::Types::todo(), std::move(value), core::Names::assertType(),
                                               std::move(type));
    }

    static ExpressionPtr NoReturn(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::noreturn(), loc.copyWithZeroLength());
    }

    static ExpressionPtr SelfType(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::selfType(), loc.copyWithZeroLength());
    }

    static ExpressionPtr Unsafe(core::LocOffsets loc, ExpressionPtr inner) {
        return Send1(loc, T(loc), core::Names::unsafe(), loc, std::move(inner));
    }

    static ExpressionPtr Untyped(core::LocOffsets loc) {
        return Send0(loc, T(loc), core::Names::untyped(), loc);
    }

    static ExpressionPtr UntypedNil(core::LocOffsets loc) {
        return Unsafe(loc, Nil(loc));
    }

    static ExpressionPtr Nilable(core::LocOffsets loc, ExpressionPtr arg) {
        return Send1(loc, T(loc), core::Names::nilable(), loc, std::move(arg));
    }

    static ExpressionPtr T_Array(core::LocOffsets loc) {
        return UnresolvedConstantParts(loc, {core::Names::Constants::T(), core::Names::Constants::Array()});
    }

    static ExpressionPtr T_Boolean(core::LocOffsets loc) {
        static constexpr core::NameRef parts[2] = {core::Names::Constants::T(), core::Names::Constants::Boolean()};
        return UnresolvedConstantParts(loc, parts);
    }

    static ExpressionPtr T_Class(core::LocOffsets loc) {
        return UnresolvedConstantParts(loc, {core::Names::Constants::T(), core::Names::Constants::Class()});
    }

    static ExpressionPtr T_Hash(core::LocOffsets loc) {
        return UnresolvedConstantParts(loc, {core::Names::Constants::T(), core::Names::Constants::Hash()});
    }

    static ExpressionPtr T_Enumerable(core::LocOffsets loc) {
        return UnresolvedConstantParts(loc, {core::Names::Constants::T(), core::Names::Constants::Enumerable()});
    }

    static ExpressionPtr T_Enumerator(core::LocOffsets loc) {
        return UnresolvedConstantParts(loc, {core::Names::Constants::T(), core::Names::Constants::Enumerator()});
    }

    static ExpressionPtr T_Enumerator_Lazy(core::LocOffsets loc) {
        return UnresolvedConstantParts(
            loc, {core::Names::Constants::T(), core::Names::Constants::Enumerator(), core::Names::Constants::Lazy()});
    }

    static ExpressionPtr T_Enumerator_Chain(core::LocOffsets loc) {
        return UnresolvedConstantParts(
            loc, {core::Names::Constants::T(), core::Names::Constants::Enumerator(), core::Names::Constants::Chain()});
    }

    static ExpressionPtr T_Proc(core::LocOffsets loc, Send::ARGS_store args, ExpressionPtr ret) {
        auto proc = Send0(loc, T(loc), core::Names::proc(), loc.copyWithZeroLength());
        auto params = Params(loc, std::move(proc), std::move(args));
        return Send1(loc, std::move(params), core::Names::returns(), loc.copyWithZeroLength(), std::move(ret));
    }

    static ExpressionPtr T_ProcVoid(core::LocOffsets loc, Send::ARGS_store args) {
        auto proc = Send0(loc, T(loc), core::Names::proc(), loc.copyWithZeroLength());
        auto params = Params(loc, std::move(proc), std::move(args));
        return Send0(loc, std::move(params), core::Names::void_(), loc.copyWithZeroLength());
    }

    static ExpressionPtr T_Range(core::LocOffsets loc) {
        return UnresolvedConstantParts(loc, {core::Names::Constants::T(), core::Names::Constants::Range()});
    }

    static ExpressionPtr T_Set(core::LocOffsets loc) {
        return UnresolvedConstantParts(loc, {core::Names::Constants::T(), core::Names::Constants::Set()});
    }

    static ExpressionPtr ZSuper(core::LocOffsets loc, core::NameRef method) {
        Send::Flags flags;
        flags.isPrivateOk = true;
        return Send(loc, Self(loc), method, loc, 1,
                    SendArgs(make_expression<ast::ZSuperArgs>(loc.copyEndWithZeroLength())), flags);
    }

    static ExpressionPtr Magic(core::LocOffsets loc) {
        return Constant(loc, core::Symbols::Magic());
    }

    static ExpressionPtr RuntimeMethodDefinition(core::LocOffsets loc, core::NameRef name, bool isSelfMethod) {
        return make_expression<ast::RuntimeMethodDefinition>(loc, name, isSelfMethod);
    }

    static ExpressionPtr RaiseUnimplemented(core::LocOffsets loc) {
        auto kernel = Constant(loc, core::Symbols::Kernel());
        auto msg = String(loc, core::Names::rewriterRaiseUnimplemented());
        // T.unsafe so that Sorbet doesn't know this unconditionally raises (avoids introducing dead code errors)
        auto ret = Send1(loc, Unsafe(loc, std::move(kernel)), core::Names::raise(), loc, std::move(msg));
        cast_tree<ast::Send>(ret)->flags.isRewriterSynthesized = true;
        return ret;
    }

    static ExpressionPtr RaiseTypedUnimplemented(core::LocOffsets loc) {
        auto kernel = Constant(loc, core::Symbols::Kernel());
        auto msg = String(loc, core::Names::rewriterRaiseUnimplemented());
        auto ret = Send1(loc, std::move(kernel), core::Names::raise(), loc, std::move(msg));
        cast_tree<ast::Send>(ret)->flags.isRewriterSynthesized = true;
        return ret;
    }

    static bool isRootScope(const ast::ExpressionPtr &scope) {
        if (ast::isa_tree<ast::EmptyTree>(scope)) {
            return true;
        }
        auto root = ast::cast_tree<ast::ConstantLit>(scope);
        return root != nullptr && root->symbol() == core::Symbols::root();
    }

    static bool isMagicClass(ExpressionPtr &expr) {
        if (auto recv = cast_tree<ConstantLit>(expr)) {
            return recv->symbol() == core::Symbols::Magic();
        } else {
            return false;
        }
    }

    static bool isSorbetPrivateStatic(const ast::ExpressionPtr &expr) {
        if (auto recv = cast_tree<ConstantLit>(expr)) {
            return recv->symbol() == core::Symbols::Sorbet_Private_Static();
        }

        return false;
    }

    static bool isSelfNew(ast::Send *send) {
        return send->fun == core::Names::new_() && send->recv.isSelfReference();
    }

    /*
     * Is this an expression that refers to resolved or unresolved `::T` constant?
     *
     * When considering unresolved `::T`, we only consider `::T` with no scope (i.e. `T`) and `::T` with the root
     * scope (i.e. `::T`). This might not actually refer to the `T` that we define for users, but we don't know that
     * information at the AST level.
     */
    static bool isT(const ast::ExpressionPtr &expr) {
        bool result = false;

        typecase(
            expr,
            [&](const ast::UnresolvedConstantLit &t) {
                // When the `T` was written by the user, we get an UnresolvedConstantLit.
                result = t.cnst == core::Names::Constants::T() && ast::MK::isRootScope(t.scope);
            },
            [&](const ast::ConstantLit &c) {
                // When the `T` was inserted by `ast::MK::T()`, we get a ConstantLit.
                result = c.symbol() == core::Symbols::T();
            },
            [&](const ast::ExpressionPtr &e) { result = false; });

        return result;
    }

    static bool isTNilable(const ast::ExpressionPtr &expr) {
        auto nilable = ast::cast_tree<ast::Send>(expr);
        return nilable != nullptr && nilable->fun == core::Names::nilable() && isT(nilable->recv);
    }

    static bool isTUntyped(const ast::ExpressionPtr &expr) {
        auto send = ast::cast_tree<ast::Send>(expr);
        return send != nullptr && send->fun == core::Names::untyped() && isT(send->recv);
    }

    static core::NameRef arg2Name(const ExpressionPtr &arg) {
        auto *cursor = &arg;
        while (true) {
            if (auto local = cast_tree<UnresolvedIdent>(*cursor)) {
                ENFORCE(local->kind == UnresolvedIdent::Kind::Local);
                return local->name;
            }

            // Recurse into structure to find the UnresolvedIdent
            typecase(
                *cursor, [&](const class RestArg &rest) { cursor = &rest.expr; },
                [&](const class KeywordArg &kw) { cursor = &kw.expr; },
                [&](const class OptionalArg &opt) { cursor = &opt.expr; },
                [&](const class BlockArg &blk) { cursor = &blk.expr; },
                [&](const class ShadowArg &shadow) { cursor = &shadow.expr; },
                // ENFORCES are last so that we don't pay the price of casting in the fast path.
                [&](const ast::Local &opt) { ENFORCE(false, "Should only be called before local_vars.cc"); },
                [&](const ExpressionPtr &expr) { ENFORCE(false, "Unexpected node type in argument position."); });
        }
    }

    static ast::Local const *arg2Local(const ast::ExpressionPtr &arg) {
        auto *cursor = &arg;
        while (true) {
            if (auto local = ast::cast_tree<ast::Local>(*cursor)) {
                // Buried deep within every argument is a Local
                return local;
            }

            // Recurse into structure to find the Local
            typecase(
                *cursor, [&](const class RestArg &rest) { cursor = &rest.expr; },
                [&](const class KeywordArg &kw) { cursor = &kw.expr; },
                [&](const class OptionalArg &opt) { cursor = &opt.expr; },
                [&](const class BlockArg &blk) { cursor = &blk.expr; },
                [&](const class ShadowArg &shadow) { cursor = &shadow.expr; },
                // ENFORCES are last so that we don't pay the price of casting in the fast path.
                [&](const UnresolvedIdent &opt) { ENFORCE(false, "Namer should have created a Local for this arg."); },
                [&](const ExpressionPtr &expr) { ENFORCE(false, "Unexpected node type in argument position."); });
        }
    }
};

class BehaviorHelpers final {
public:
    // Recursively check if all children of an expression are EmptyTree's or InsSeq's that only contain EmptyTree's
    static bool checkEmptyDeep(const ExpressionPtr &);

    // Does a class/module definition define "behavior"? A class definition that only serves as a
    // namespace for inner-definitions is not considered to have behavior.
    //
    // module A
    //   CONST = true <-- Behavior on A::CONST (not A)
    //   module B
    //     def m; end <-- Behavior in A::B
    //   end
    // end
    static bool checkClassDefinesBehavior(const ExpressionPtr &);
    static bool checkClassDefinesBehavior(const ast::ClassDef &);
};

} // namespace sorbet::ast

#endif // SORBET_TREES_H
