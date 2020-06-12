#include "resolver/type_syntax.h"
#include "absl/strings/str_join.h"
#include "common/typecase.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/core.h"
#include "core/errors/resolver.h"

using namespace std;

namespace sorbet::resolver {

// Forward declarations for the local versions of getResultType, getResultTypeAndBind, and parseSig that skolemize type
// members.
namespace {
core::TypePtr getResultTypeWithSelfTypeParams(core::MutableContext ctx, ast::TreePtr &expr,
                                              const ParsedSig &sigBeingParsed, TypeSyntaxArgs args);

TypeSyntax::ResultType getResultTypeAndBindWithSelfTypeParams(core::MutableContext ctx, ast::TreePtr &expr,
                                                              const ParsedSig &sigBeingParsed, TypeSyntaxArgs args);

ParsedSig parseSigWithSelfTypeParams(core::MutableContext ctx, ast::Send *sigSend, const ParsedSig *parent,
                                     TypeSyntaxArgs args);
} // namespace

ParsedSig TypeSyntax::parseSig(core::MutableContext ctx, ast::Send *sigSend, const ParsedSig *parent,
                               TypeSyntaxArgs args) {
    auto result = parseSigWithSelfTypeParams(ctx, sigSend, parent, args);

    for (auto &arg : result.argTypes) {
        arg.type = core::Types::unwrapSelfTypeParam(ctx, arg.type);
    }

    if (result.returns != nullptr) {
        result.returns = core::Types::unwrapSelfTypeParam(ctx, result.returns);
    }

    return result;
}

core::TypePtr TypeSyntax::getResultType(core::MutableContext ctx, ast::TreePtr &expr, const ParsedSig &sigBeingParsed,
                                        TypeSyntaxArgs args) {
    return core::Types::unwrapSelfTypeParam(
        ctx, getResultTypeWithSelfTypeParams(ctx, expr, sigBeingParsed, args.withoutRebind()));
}

TypeSyntax::ResultType TypeSyntax::getResultTypeAndBind(core::MutableContext ctx, ast::TreePtr &expr,
                                                        const ParsedSig &sigBeingParsed, TypeSyntaxArgs args) {
    auto result = getResultTypeAndBindWithSelfTypeParams(ctx, expr, sigBeingParsed, args);
    result.type = core::Types::unwrapSelfTypeParam(ctx, result.type);
    return result;
}

core::TypePtr getResultLiteral(core::Context ctx, ast::TreePtr &expr) {
    core::TypePtr result;
    typecase(
        expr.get(), [&](ast::Literal *lit) { result = lit->value; },
        [&](ast::Expression *expr) {
            if (auto e = ctx.beginError(expr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported type literal");
            }
            result = core::Types::untypedUntracked();
        });
    ENFORCE(result.get() != nullptr);
    result->sanityCheck(ctx);
    return result;
}

bool isTProc(core::Context ctx, ast::Send *send) {
    while (send != nullptr) {
        if (send->fun == core::Names::proc()) {
            if (auto *rcv = ast::cast_tree<ast::ConstantLit>(send->recv)) {
                return rcv->symbol == core::Symbols::T();
            }
        }
        send = ast::cast_tree<ast::Send>(send->recv);
    }
    return false;
}

bool TypeSyntax::isSig(core::Context ctx, ast::Send *send) {
    if (send->fun != core::Names::sig()) {
        return false;
    }
    if (send->block.get() == nullptr) {
        return false;
    }
    auto nargs = send->args.size();
    if (!(nargs == 1 || nargs == 2)) {
        return false;
    }

    auto recv = ast::cast_tree<ast::ConstantLit>(send->recv);
    if (recv && recv->symbol == core::Symbols::Sorbet_Private_Static()) {
        return true;
    }

    return false;
}

namespace {

ParsedSig parseSigWithSelfTypeParams(core::MutableContext ctx, ast::Send *sigSend, const ParsedSig *parent,
                                     TypeSyntaxArgs args) {
    ParsedSig sig;

    vector<ast::Send *> sends;

    if (isTProc(ctx, sigSend)) {
        sends.emplace_back(sigSend);
    } else {
        sig.seen.sig = true;
        ENFORCE(sigSend->fun == core::Names::sig());
        auto block = ast::cast_tree<ast::Block>(sigSend->block);
        ENFORCE(block);
        auto send = ast::cast_tree<ast::Send>(block->body);
        if (send) {
            sends.emplace_back(send);
        } else {
            auto insseq = ast::cast_tree<ast::InsSeq>(block->body);
            if (insseq) {
                for (auto &stat : insseq->stats) {
                    send = ast::cast_tree<ast::Send>(stat);
                    if (!send) {
                        return sig;
                    }
                    sends.emplace_back(send);
                }
                send = ast::cast_tree<ast::Send>(insseq->expr);
                if (!send) {
                    return sig;
                }
                sends.emplace_back(send);
            } else {
                return sig;
            }
        }
    }
    ENFORCE(!sends.empty());

    if (sigSend->args.size() == 2) {
        auto lit = ast::cast_tree<ast::Literal>(sigSend->args[1]);
        if (lit != nullptr && lit->isSymbol(ctx) && lit->asSymbol(ctx) == core::Names::final_()) {
            sig.seen.final = true;
        }
    }

    for (auto &send : sends) {
        ast::Send *tsend = send;
        // extract type parameters early
        while (tsend != nullptr) {
            if (tsend->fun == core::Names::typeParameters()) {
                if (parent != nullptr) {
                    if (auto e = ctx.beginError(tsend->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed signature; Type parameters can only be specified in outer sig");
                    }
                    break;
                }
                for (auto &arg : tsend->args) {
                    if (auto c = ast::cast_tree<ast::Literal>(arg)) {
                        if (c->isSymbol(ctx)) {
                            auto name = c->asSymbol(ctx);
                            auto &typeArgSpec = sig.enterTypeArgByName(name);
                            if (typeArgSpec.type) {
                                if (auto e = ctx.beginError(arg->loc, core::errors::Resolver::InvalidMethodSignature)) {
                                    e.setHeader("Malformed signature; Type argument `{}` was specified twice",
                                                name.show(ctx));
                                }
                            }
                            typeArgSpec.type = core::make_type<core::TypeVar>(core::Symbols::todo());
                            typeArgSpec.loc = core::Loc(ctx.file, arg->loc);
                        } else {
                            if (auto e = ctx.beginError(arg->loc, core::errors::Resolver::InvalidMethodSignature)) {
                                e.setHeader("Malformed signature; Type parameters are specified with symbols");
                            }
                        }
                    } else {
                        if (auto e = ctx.beginError(arg->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Malformed signature; Type parameters are specified with symbols");
                        }
                    }
                }
            }
            tsend = ast::cast_tree<ast::Send>(tsend->recv);
        }
    }
    if (parent == nullptr) {
        parent = &sig;
    }

    for (auto &send : sends) {
        while (send != nullptr) {
            // so we don't report multiple "method does not exist" errors arising from the same expression
            bool reportedInvalidMethod = false;
            switch (send->fun._id) {
                case core::Names::proc()._id:
                    sig.seen.proc = true;
                    break;
                case core::Names::bind()._id: {
                    if (sig.seen.bind) {
                        if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Malformed `{}`: Multiple calls to `.bind`", send->fun.show(ctx));
                        }
                        sig.bind = core::SymbolRef();
                    }
                    sig.seen.bind = true;

                    if (send->args.size() != 1) {
                        if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", "bind", 1,
                                        send->args.size());
                        }
                        break;
                    }

                    bool validBind = false;
                    auto bind = getResultTypeWithSelfTypeParams(ctx, send->args.front(), *parent, args);
                    if (auto classType = core::cast_type<core::ClassType>(bind.get())) {
                        sig.bind = classType->symbol;
                        validBind = true;
                    } else if (auto appType = core::cast_type<core::AppliedType>(bind.get())) {
                        // When `T.proc.bind` is used with `T.class_of`, pass it
                        // through as long as it only has the AttachedClass type
                        // member.
                        if (appType->klass.data(ctx)->isSingletonClass(ctx) &&
                            appType->klass.data(ctx)->typeMembers().size() == 1) {
                            sig.bind = appType->klass;
                            validBind = true;
                        }
                    }

                    if (!validBind) {
                        if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Malformed `{}`: Can only bind to simple class names", send->fun.show(ctx));
                        }
                    }

                    break;
                }
                case core::Names::params()._id: {
                    if (sig.seen.params) {
                        if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Malformed `{}`: Multiple calls to `.params`", send->fun.show(ctx));
                        }
                        sig.argTypes.clear();
                    }
                    sig.seen.params = true;

                    if (send->args.empty()) {
                        break;
                    }

                    if (send->args.size() > 1) {
                        if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", send->fun.show(ctx),
                                        "0-1", send->args.size());
                        }
                    }

                    auto *hash = ast::cast_tree<ast::Hash>(send->args[0]);
                    if (hash == nullptr) {
                        if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            auto paramsStr = send->fun.show(ctx);
                            e.setHeader("`{}` expects keyword arguments", paramsStr);
                            e.addErrorSection(core::ErrorSection(core::ErrorColors::format(
                                "All parameters must be given names in `{}` even if they are positional", paramsStr)));
                        }
                        break;
                    }

                    int i = 0;
                    for (auto &key : hash->keys) {
                        auto &value = hash->values[i++];
                        auto lit = ast::cast_tree<ast::Literal>(key);
                        if (lit && lit->isSymbol(ctx)) {
                            core::NameRef name = lit->asSymbol(ctx);
                            auto resultAndBind =
                                getResultTypeAndBindWithSelfTypeParams(ctx, value, *parent, args.withRebind());
                            sig.argTypes.emplace_back(ParsedSig::ArgSpec{core::Loc(ctx.file, key->loc), name,
                                                                         resultAndBind.type, resultAndBind.rebind});
                        }
                    }
                    break;
                }
                case core::Names::typeParameters()._id:
                    // was handled above
                    break;
                case core::Names::abstract()._id:
                    sig.seen.abstract = true;
                    break;
                case core::Names::override_()._id: {
                    sig.seen.override_ = true;

                    if (send->args.size() > 1) {
                        if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", send->fun.show(ctx),
                                        "0-1", send->args.size());
                        }
                    }

                    if (send->args.size() == 1) {
                        auto *hash = ast::cast_tree<ast::Hash>(send->args[0]);
                        if (hash == nullptr) {
                            if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                                auto paramsStr = send->fun.show(ctx);
                                e.setHeader("`{}` expects keyword arguments", send->fun.show(ctx));
                            }
                            break;
                        }

                        int i = 0;
                        for (auto &key : hash->keys) {
                            auto &value = hash->values[i++];
                            auto lit = ast::cast_tree<ast::Literal>(key);
                            if (lit && lit->isSymbol(ctx)) {
                                if (lit->asSymbol(ctx) == core::Names::allowIncompatible()) {
                                    auto val = ast::cast_tree<ast::Literal>(value);
                                    if (val && val->isTrue(ctx)) {
                                        sig.seen.incompatibleOverride = true;
                                    }
                                }
                            }
                        }
                    }

                    break;
                }
                case core::Names::implementation()._id:
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::ImplementationDeprecated)) {
                        e.setHeader("Use of `{}` has been replaced by `{}`", "implementation", "override");
                        if (send->recv->isSelfReference()) {
                            e.replaceWith("Replace with `override`", core::Loc(ctx.file, send->loc), "override");
                        } else {
                            e.replaceWith("Replace with `override`", core::Loc(ctx.file, send->loc), "{}.override",
                                          core::Loc(ctx.file, send->recv->loc).source(ctx));
                        }
                    }
                    break;
                case core::Names::overridable()._id:
                    sig.seen.overridable = true;
                    break;
                case core::Names::returns()._id: {
                    sig.seen.returns = true;
                    if (send->args.size() != 1) {
                        if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", "returns", 1,
                                        send->args.size());
                        }
                        break;
                    }

                    auto nil = ast::cast_tree<ast::Literal>(send->args[0]);
                    if (nil && nil->isNil(ctx)) {
                        const auto loc = core::Loc(ctx.file, send->args[0]->loc);
                        if (auto e = ctx.state.beginError(loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("You probably meant `.returns(NilClass)`");
                            e.replaceWith("Replace with `NilClass`", loc, "NilClass");
                        }
                        sig.returns = core::Types::nilClass();
                        break;
                    }

                    sig.returns = getResultTypeWithSelfTypeParams(ctx, send->args.front(), *parent, args);

                    break;
                }
                case core::Names::void_()._id:
                    sig.seen.void_ = true;
                    sig.returns = core::Types::void_();
                    break;
                case core::Names::checked()._id:
                    sig.seen.checked = true;
                    break;
                case core::Names::onFailure()._id:
                    break;
                case core::Names::final_()._id:
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        reportedInvalidMethod = true;
                        e.setHeader("The syntax for declaring a method final is `sig(:final) {{...}}`, not `sig "
                                    "{{final. ...}}`");
                    }
                    break;
                default:
                    if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        reportedInvalidMethod = true;
                        e.setHeader("Malformed signature: `{}` is invalid in this context", send->fun.show(ctx));
                        e.addErrorLine(core::Loc(ctx.file, send->loc),
                                       "Consult https://sorbet.org/docs/sigs for signature syntax");
                    }
            }
            auto recv = ast::cast_tree<ast::Send>(send->recv);

            // we only report this error if we haven't reported another unknown method error
            if (!recv && !reportedInvalidMethod) {
                if (!send->recv->isSelfReference()) {
                    if (!sig.seen.proc) {
                        if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Malformed signature: `{}` being invoked on an invalid receiver",
                                        send->fun.show(ctx));
                        }
                    }
                }
                break;
            }

            send = recv;
        }
    }
    ENFORCE(sig.seen.sig || sig.seen.proc);

    return sig;
}

core::TypePtr interpretTCombinator(core::MutableContext ctx, ast::Send *send, const ParsedSig &sig,
                                   TypeSyntaxArgs args) {
    switch (send->fun._id) {
        case core::Names::nilable()._id:
            if (send->args.size() != 1) {
                return core::Types::untypedUntracked(); // error will be reported in infer.
            }
            return core::Types::any(ctx, getResultTypeWithSelfTypeParams(ctx, send->args[0], sig, args),
                                    core::Types::nilClass());
        case core::Names::all()._id: {
            if (send->args.empty()) {
                // Error will be reported in infer
                return core::Types::untypedUntracked();
            }
            auto result = getResultTypeWithSelfTypeParams(ctx, send->args[0], sig, args);
            int i = 1;
            while (i < send->args.size()) {
                result = core::Types::all(ctx, result, getResultTypeWithSelfTypeParams(ctx, send->args[i], sig, args));
                i++;
            }
            return result;
        }
        case core::Names::any()._id: {
            if (send->args.empty()) {
                // Error will be reported in infer
                return core::Types::untypedUntracked();
            }
            auto result = getResultTypeWithSelfTypeParams(ctx, send->args[0], sig, args);
            int i = 1;
            while (i < send->args.size()) {
                result = core::Types::any(ctx, result, getResultTypeWithSelfTypeParams(ctx, send->args[i], sig, args));
                i++;
            }
            return result;
        }
        case core::Names::typeParameter()._id: {
            if (send->args.size() != 1) {
                // Error will be reported in infer
                return core::Types::untypedUntracked();
            }
            auto arr = ast::cast_tree<ast::Literal>(send->args[0]);
            if (!arr || !arr->isSymbol(ctx)) {
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("type_parameter requires a symbol");
                }
                return core::Types::untypedUntracked();
            }
            auto fnd = sig.findTypeArgByName(arr->asSymbol(ctx));
            if (!fnd.type) {
                if (auto e = ctx.beginError(arr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Unspecified type parameter");
                }
                return core::Types::untypedUntracked();
            }
            return fnd.type;
        }
        case core::Names::enum_()._id: {
            if (send->args.size() != 1) {
                // Error will be reported in infer
                return core::Types::untypedUntracked();
            }
            auto arr = ast::cast_tree<ast::Array>(send->args[0]);
            if (arr == nullptr) {
                // TODO(pay-server) unsilence this error and support enums from pay-server
                { return core::Types::Object(); }
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("enum must be passed a literal array. e.g. enum([1,\"foo\",MyClass])");
                }
                return core::Types::untypedUntracked();
            }
            if (arr->elems.empty()) {
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("enum([]) is invalid");
                }
                return core::Types::untypedUntracked();
            }
            auto result = getResultLiteral(ctx, arr->elems[0]);
            int i = 1;
            while (i < arr->elems.size()) {
                result = core::Types::any(ctx, result, getResultLiteral(ctx, arr->elems[i]));
                i++;
            }
            return result;
        }
        case core::Names::classOf()._id: {
            if (send->args.size() != 1) {
                // Error will be reported in infer
                return core::Types::untypedUntracked();
            }

            auto *obj = ast::cast_tree<ast::ConstantLit>(send->args[0]);
            if (!obj) {
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.class_of needs a Class as its argument");
                }
                return core::Types::untypedUntracked();
            }
            auto maybeAliased = obj->symbol;
            if (maybeAliased.data(ctx)->isTypeAlias()) {
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.class_of can't be used with a T.type_alias");
                }
                return core::Types::untypedUntracked();
            }
            if (maybeAliased.data(ctx)->isTypeMember()) {
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.class_of can't be used with a T.type_member");
                }
                return core::Types::untypedUntracked();
            }
            auto sym = maybeAliased.data(ctx)->dealias(ctx);
            if (sym.data(ctx)->isStaticField()) {
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.class_of can't be used with a constant field");
                }
                return core::Types::untypedUntracked();
            }

            auto singleton = sym.data(ctx)->singletonClass(ctx);
            if (!singleton.exists()) {
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Unknown class");
                }
                return core::Types::untypedUntracked();
            }
            return singleton.data(ctx)->externalType(ctx);
        }
        case core::Names::untyped()._id:
            return core::Types::untyped(ctx, args.untypedBlame);
        case core::Names::selfType()._id:
            if (args.allowSelfType) {
                return core::make_type<core::SelfType>();
            }
            if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Only top-level T.self_type is supported");
            }
            return core::Types::untypedUntracked();
        case core::Names::experimentalAttachedClass()._id:
        case core::Names::attachedClass()._id:
            if (send->fun == core::Names::experimentalAttachedClass()) {
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::ExperimentalAttachedClass)) {
                    e.setHeader("`{}` has been stabilized and is no longer experimental",
                                "T.experimental_attached_class");
                    e.replaceWith("Replace with `T.attached_class`", core::Loc(ctx.file, send->loc),
                                  "T.attached_class");
                }
            }

            if (!ctx.owner.data(ctx)->isSingletonClass(ctx)) {
                if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("`{}` may only be used in a singleton class method context",
                                "T." + core::Names::attachedClass().show(ctx));
                }
                return core::Types::untypedUntracked();
            } else {
                // All singletons have an AttachedClass type member, created by
                // `singletonClass`
                auto attachedClass = ctx.owner.data(ctx)->findMember(ctx, core::Names::Constants::AttachedClass());
                return attachedClass.data(ctx)->resultType;
            }
        case core::Names::noreturn()._id:
            return core::Types::bottom();

        default:
            if (auto e = ctx.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported method `{}`", "T." + send->fun.show(ctx));
            }
            return core::Types::untypedUntracked();
    }
}

core::TypePtr getResultTypeWithSelfTypeParams(core::MutableContext ctx, ast::TreePtr &expr,
                                              const ParsedSig &sigBeingParsed, TypeSyntaxArgs args) {
    return getResultTypeAndBindWithSelfTypeParams(ctx, expr, sigBeingParsed, args.withoutRebind()).type;
}

TypeSyntax::ResultType getResultTypeAndBindWithSelfTypeParams(core::MutableContext ctx, ast::TreePtr &expr,
                                                              const ParsedSig &sigBeingParsed, TypeSyntaxArgs args) {
    // Ensure that we only check types from a class context
    auto ctxOwnerData = ctx.owner.data(ctx);
    ENFORCE(ctxOwnerData->isClassOrModule(), "getResultTypeAndBind wasn't called with a class owner");

    TypeSyntax::ResultType result;
    typecase(
        expr.get(),
        [&](ast::Array *arr) {
            vector<core::TypePtr> elems;
            for (auto &el : arr->elems) {
                elems.emplace_back(getResultTypeWithSelfTypeParams(ctx, el, sigBeingParsed, args.withoutSelfType()));
            }
            result.type = core::TupleType::build(ctx, elems);
        },
        [&](ast::Hash *hash) {
            vector<core::TypePtr> keys;
            vector<core::TypePtr> values;

            for (auto &ktree : hash->keys) {
                auto &vtree = hash->values[&ktree - &hash->keys.front()];
                auto val = getResultTypeWithSelfTypeParams(ctx, vtree, sigBeingParsed, args.withoutSelfType());
                auto lit = ast::cast_tree<ast::Literal>(ktree);
                if (lit && (lit->isSymbol(ctx) || lit->isString(ctx))) {
                    ENFORCE(core::cast_type<core::LiteralType>(lit->value.get()));
                    keys.emplace_back(lit->value);
                    values.emplace_back(val);
                } else {
                    if (auto e = ctx.beginError(ktree->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed type declaration. Shape keys must be literals");
                    }
                }
            }
            result.type = core::make_type<core::ShapeType>(core::Types::hashOfUntyped(), keys, values);
        },
        [&](ast::ConstantLit *i) {
            auto maybeAliased = i->symbol;
            ENFORCE(maybeAliased.exists());

            if (maybeAliased.data(ctx)->isTypeAlias()) {
                result.type = maybeAliased.data(ctx)->resultType;
                return;
            }

            // Only T::Enum singletons are allowed to be in type syntax because there is only one
            // non-alias constant for an instance of a particular T::Enum--it was created with
            // `new` and immediately assigned into a constant. Any further ConstantLits of this enum's
            // type must be either class aliases (banned in type syntax) or type aliases.
            //
            // This is not the case for arbitrary singletons: MySingleton.instance can be called as many
            // times as wanted, and assigned into different constants each time. As much as possible, we
            // want there to be one name for every type; making an alias for a type should always be
            // syntactically declared with T.type_alias.
            if (auto resultType = core::cast_type<core::ClassType>(maybeAliased.data(ctx)->resultType.get())) {
                if (resultType->symbol.data(ctx)->derivesFrom(ctx, core::Symbols::T_Enum())) {
                    result.type = maybeAliased.data(ctx)->resultType;
                    return;
                }
            }

            auto sym = maybeAliased.data(ctx)->dealias(ctx);
            if (sym.data(ctx)->isClassOrModule()) {
                // the T::Type generics internally have a typeArity of 0, so this allows us to check against them in the
                // same way that we check against types like `Array`
                bool isBuiltinGeneric = sym == core::Symbols::T_Hash() || sym == core::Symbols::T_Array() ||
                                        sym == core::Symbols::T_Set() || sym == core::Symbols::T_Range() ||
                                        sym == core::Symbols::T_Enumerable() || sym == core::Symbols::T_Enumerator();

                if (isBuiltinGeneric || sym.data(ctx)->typeArity(ctx) > 0) {
                    // This set **should not** grow over time.
                    bool isStdlibWhitelisted = sym == core::Symbols::Hash() || sym == core::Symbols::Array() ||
                                               sym == core::Symbols::Set() || sym == core::Symbols::Range() ||
                                               sym == core::Symbols::Enumerable() || sym == core::Symbols::Enumerator();
                    auto level = isStdlibWhitelisted ? core::errors::Resolver::GenericClassWithoutTypeArgsStdlib
                                                     : core::errors::Resolver::GenericClassWithoutTypeArgs;
                    if (auto e = ctx.beginError(i->loc, level)) {
                        e.setHeader("Malformed type declaration. Generic class without type arguments `{}`",
                                    sym.show(ctx));
                        // if we're looking at `Array`, we want the autocorrect to include `T::`, but we don't need to
                        // if we're already looking at `T::Array` instead.
                        auto typePrefix = isBuiltinGeneric ? "" : "T::";
                        if (sym == core::Symbols::Hash() || sym == core::Symbols::T_Hash()) {
                            // Hash is special because it has arity 3 but you're only supposed to write the first 2
                            e.replaceWith("Add type arguments", core::Loc(ctx.file, i->loc),
                                          "{}{}[T.untyped, T.untyped]", typePrefix,
                                          core::Loc(ctx.file, i->loc).source(ctx));
                        } else if (isStdlibWhitelisted || isBuiltinGeneric) {
                            // the default provided here for builtin generic types is 1, and that might need to change
                            // if we add other builtin generics (but ideally we should never need to do so!)
                            auto numTypeArgs = isBuiltinGeneric ? 1 : sym.data(ctx)->typeArity(ctx);
                            vector<string> untypeds;
                            for (int i = 0; i < numTypeArgs; i++) {
                                untypeds.emplace_back("T.untyped");
                            }
                            e.replaceWith("Add type arguments", core::Loc(ctx.file, i->loc), "{}{}[{}]", typePrefix,
                                          core::Loc(ctx.file, i->loc).source(ctx), absl::StrJoin(untypeds, ", "));
                        }
                    }
                }
                if (sym == core::Symbols::StubModule()) {
                    // Though for normal types _and_ stub types `infer` should use `externalType`,
                    // using `externalType` for stub types here will lead to incorrect handling of global state hashing,
                    // where we won't see difference between two different unresolved stubs(or a mistyped stub). thus,
                    // while normally we would treat stubs as untyped, in `sig`s we treat them as proper types, so that
                    // we can correctly hash them.
                    auto unresolvedPath = i->fullUnresolvedPath(ctx);
                    ENFORCE(unresolvedPath.has_value());
                    result.type =
                        core::make_type<core::UnresolvedClassType>(unresolvedPath->first, move(unresolvedPath->second));
                } else {
                    result.type = sym.data(ctx)->externalType(ctx);
                }
            } else if (sym.data(ctx)->isTypeMember()) {
                auto symData = sym.data(ctx);
                auto symOwner = symData->owner.data(ctx);

                bool isTypeTemplate = symOwner->isSingletonClass(ctx);

                if (args.allowTypeMember) {
                    bool ctxIsSingleton = ctxOwnerData->isSingletonClass(ctx);

                    // Check if we're processing a type within the class that
                    // defines this type member by comparing the singleton class of
                    // the context, and the singleton class of the type member's
                    // owner.
                    core::SymbolRef symOwnerSingleton =
                        isTypeTemplate ? symData->owner : symOwner->lookupSingletonClass(ctx);
                    core::SymbolRef ctxSingleton = ctxIsSingleton ? ctx.owner : ctxOwnerData->lookupSingletonClass(ctx);
                    bool usedOnSourceClass = symOwnerSingleton == ctxSingleton;

                    // For this to be a valid use of a member or template type, this
                    // must:
                    //
                    // 1. be used in the context of the class that defines it
                    // 2. if it's a type_template type, be used in a singleton
                    //    method
                    // 3. if it's a type_member type, be used in an instance method
                    if (usedOnSourceClass &&
                        ((isTypeTemplate && ctxIsSingleton) || !(isTypeTemplate || ctxIsSingleton))) {
                        // At this point, we maake a skolemized variable that will be unwrapped at the end of type
                        // parsing using Types::unwrapSkolemVariables. The justification for this is that type
                        // constructors like `Types::any` do not expect to see bound variables, and will panic.
                        result.type = core::make_type<core::SelfTypeParam>(sym);
                    } else {
                        if (auto e = ctx.beginError(i->loc, core::errors::Resolver::InvalidTypeDeclarationTyped)) {
                            string typeSource = isTypeTemplate ? "type_template" : "type_member";
                            string typeStr = sym.show(ctx);

                            if (usedOnSourceClass) {
                                if (ctxIsSingleton) {
                                    e.setHeader("`{}` type `{}` used in a singleton method definition", typeSource,
                                                typeStr);
                                } else {
                                    e.setHeader("`{}` type `{}` used in an instance method definition", typeSource,
                                                typeStr);
                                }
                            } else {
                                e.setHeader("`{}` type `{}` used outside of the class definition", typeSource, typeStr);
                            }
                        }
                        result.type = core::Types::untypedUntracked();
                    }
                } else {
                    // a type member has occurred in a context that doesn't allow them
                    if (auto e = ctx.beginError(i->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        auto flavor = isTypeTemplate ? "type_template"sv : "type_member"sv;
                        e.setHeader("`{}` `{}` is not allowed in this context", flavor, sym.show(ctx));
                    }
                    result.type = core::Types::untypedUntracked();
                }
            } else if (sym.data(ctx)->isStaticField()) {
                if (auto e = ctx.beginError(i->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Constant `{}` is not a class or type alias", maybeAliased.show(ctx));
                    e.addErrorLine(sym.data(ctx)->loc(),
                                   "If you are trying to define a type alias, you should use `{}` here",
                                   "T.type_alias");
                }
                result.type = core::Types::untypedUntracked();
            } else {
                if (auto e = ctx.beginError(i->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Not a class type `{}`", maybeAliased.show(ctx));
                }
                result.type = core::Types::untypedUntracked();
            }
        },
        [&](ast::Send *s) {
            if (isTProc(ctx, s)) {
                auto sig = parseSigWithSelfTypeParams(ctx, s, &sigBeingParsed, args.withoutSelfType());
                if (sig.bind.exists()) {
                    if (!args.allowRebind) {
                        if (auto e = ctx.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                            e.setHeader("Using `{}` is not permitted here", "bind");
                        }
                    } else {
                        result.rebind = sig.bind;
                    }
                }

                vector<core::TypePtr> targs;

                if (sig.returns == nullptr) {
                    if (auto e = ctx.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed T.proc: You must specify a return type");
                    }
                    targs.emplace_back(core::Types::untypedUntracked());
                } else {
                    targs.emplace_back(sig.returns);
                }

                for (auto &arg : sig.argTypes) {
                    targs.emplace_back(arg.type);
                }

                auto arity = targs.size() - 1;
                if (arity > core::Symbols::MAX_PROC_ARITY) {
                    if (auto e = ctx.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed T.proc: Too many arguments (max `{}`)", core::Symbols::MAX_PROC_ARITY);
                    }
                    result.type = core::Types::untypedUntracked();
                    return;
                }
                auto sym = core::Symbols::Proc(arity);

                result.type = core::make_type<core::AppliedType>(sym, targs);
                return;
            }

            auto *recvi = ast::cast_tree<ast::ConstantLit>(s->recv);
            if (recvi == nullptr) {
                if (auto e = ctx.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
                }
                result.type = core::Types::untypedUntracked();
                return;
            }
            if (recvi->symbol == core::Symbols::T()) {
                result.type = interpretTCombinator(ctx, s, sigBeingParsed, args);
                return;
            }

            if (recvi->symbol == core::Symbols::Magic() && s->fun == core::Names::callWithSplat()) {
                // TODO(pay-server) remove this block
                if (auto e = ctx.beginError(recvi->loc, core::errors::Resolver::InvalidTypeDeclarationTyped)) {
                    e.setHeader("Malformed type declaration: splats cannot be used in types");
                }
                result.type = core::Types::untypedUntracked();
                return;
            }

            if (s->fun != core::Names::squareBrackets()) {
                if (auto e = ctx.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
                }
                result.type = core::Types::untypedUntracked();
                return;
            }

            InlinedVector<unique_ptr<core::TypeAndOrigins>, 2> holders;
            InlinedVector<const core::TypeAndOrigins *, 2> targs;
            InlinedVector<core::LocOffsets, 2> argLocs;
            targs.reserve(s->args.size());
            argLocs.reserve(s->args.size());
            holders.reserve(s->args.size());
            for (auto &arg : s->args) {
                core::TypeAndOrigins ty;
                ty.origins.emplace_back(core::Loc(ctx.file, arg->loc));
                ty.type = core::make_type<core::MetaType>(
                    getResultTypeWithSelfTypeParams(ctx, arg, sigBeingParsed, args.withoutSelfType()));
                holders.emplace_back(make_unique<core::TypeAndOrigins>(move(ty)));
                targs.emplace_back(holders.back().get());
                argLocs.emplace_back(arg->loc);
            }

            core::SymbolRef corrected;
            if (recvi->symbol == core::Symbols::Array()) {
                corrected = core::Symbols::T_Array();
            } else if (recvi->symbol == core::Symbols::Hash()) {
                corrected = core::Symbols::T_Hash();
            } else if (recvi->symbol == core::Symbols::Enumerable()) {
                corrected = core::Symbols::T_Enumerable();
            } else if (recvi->symbol == core::Symbols::Enumerator()) {
                corrected = core::Symbols::T_Enumerator();
            } else if (recvi->symbol == core::Symbols::Range()) {
                corrected = core::Symbols::T_Range();
            } else if (recvi->symbol == core::Symbols::Set()) {
                corrected = core::Symbols::T_Set();
            }
            if (corrected.exists()) {
                if (auto e = ctx.beginError(s->loc, core::errors::Resolver::BadStdlibGeneric)) {
                    e.setHeader("Use `{}`, not `{}` to declare a typed `{}`", corrected.data(ctx)->show(ctx) + "[...]",
                                recvi->symbol.data(ctx)->show(ctx) + "[...]", recvi->symbol.data(ctx)->show(ctx));
                    e.addErrorSection(
                        core::ErrorSection(core::ErrorColors::format("`{}` will not work in the runtime type system.",
                                                                     recvi->symbol.data(ctx)->show(ctx) + "[...]")));
                    e.replaceWith(fmt::format("Change `{}` to `{}`", recvi->symbol.data(ctx)->show(ctx),
                                              corrected.data(ctx)->show(ctx)),
                                  core::Loc(ctx.file, recvi->loc), "{}", corrected.data(ctx)->show(ctx));
                }
                result.type = core::Types::untypedUntracked();
                return;
            } else {
                corrected = recvi->symbol;
            }
            corrected = corrected.data(ctx)->dealias(ctx);

            if (!corrected.data(ctx)->isClassOrModule()) {
                if (auto e = ctx.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Expected a class or module");
                }
                result.type = core::Types::untypedUntracked();
                return;
            }

            auto correctedSingleton = corrected.data(ctx)->singletonClass(ctx);
            auto ctype = core::make_type<core::ClassType>(correctedSingleton);
            core::CallLocs locs{
                ctx.file,
                s->loc,
                recvi->loc,
                argLocs,
            };
            core::DispatchArgs dispatchArgs{core::Names::squareBrackets(), locs, targs, ctype, ctype, nullptr};
            auto out = core::Types::dispatchCallWithoutBlock(ctx, ctype, dispatchArgs);

            if (out->isUntyped()) {
                // Using a generic untyped type here will lead to incorrect handling of global state hashing,
                // where we won't see difference between types with generic arguments.
                // Thus, while normally we would treat these as untyped, in `sig`s we treat them as proper types, so
                // that we can correctly hash them.
                vector<core::TypePtr> targPtrs;
                targPtrs.reserve(targs.size());
                for (auto &targ : targs) {
                    targPtrs.push_back(targ->type);
                }
                result.type = core::make_type<core::UnresolvedAppliedType>(correctedSingleton, move(targPtrs));
                return;
            }
            if (auto *mt = core::cast_type<core::MetaType>(out.get())) {
                result.type = mt->wrapped;
                return;
            }

            if (auto e = ctx.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
            }
            result.type = core::Types::untypedUntracked();
        },
        [&](ast::Local *slf) {
            if (slf->isSelfReference()) {
                result.type = ctxOwnerData->selfType(ctx);
            } else {
                if (auto e = ctx.beginError(slf->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Unsupported type syntax");
                }
                result.type = core::Types::untypedUntracked();
            }
        },
        [&](ast::Expression *expr) {
            if (auto e = ctx.beginError(expr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported type syntax");
            }
            result.type = core::Types::untypedUntracked();
        });
    ENFORCE(result.type.get() != nullptr);
    result.type->sanityCheck(ctx);
    return result;
}
} // namespace

ParsedSig::TypeArgSpec &ParsedSig::enterTypeArgByName(core::NameRef name) {
    for (auto &current : typeArgs) {
        if (current.name == name) {
            return current;
        }
    }
    auto &inserted = typeArgs.emplace_back();
    inserted.name = name;
    return inserted;
}

const ParsedSig::TypeArgSpec emptyTypeArgSpec;

const ParsedSig::TypeArgSpec &ParsedSig::findTypeArgByName(core::NameRef name) const {
    for (auto &current : typeArgs) {
        if (current.name == name) {
            return current;
        }
    }
    return emptyTypeArgSpec;
}
} // namespace sorbet::resolver
