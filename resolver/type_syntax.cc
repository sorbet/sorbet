#include "resolver/type_syntax.h"
#include "../core/Symbols.h"
#include "core/Names/resolver.h"
#include "core/core.h"
#include "core/errors/resolver.h"

using namespace std;

namespace sorbet {
namespace resolver {

shared_ptr<core::Type> getResultLiteral(core::MutableContext ctx, unique_ptr<ast::Expression> &expr) {
    shared_ptr<core::Type> result;
    typecase(expr.get(), [&](ast::Literal *lit) { result = lit->value; },
             [&](ast::Expression *expr) {
                 if (auto e = ctx.state.beginError(expr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                     e.setHeader("Unsupported type literal");
                 }
                 result = core::Types::untypedUntracked();
             });
    ENFORCE(result.get() != nullptr);
    result->sanityCheck(ctx);
    return result;
}

bool isTProc(core::MutableContext ctx, ast::Send *send) {
    while (send != nullptr) {
        if (send->fun == core::Names::proc()) {
            auto recv = send->recv.get();
            if (auto *rcv = ast::cast_tree<ast::ConstantLit>(recv)) {
                return rcv->symbol == core::Symbols::T();
            }
        }
        send = ast::cast_tree<ast::Send>(send->recv.get());
    }
    return false;
}

bool TypeSyntax::isSig(core::MutableContext ctx, ast::Send *send) {
    if (send->fun != core::Names::sig()) {
        return false;
    }
    if (send->block.get() == nullptr) {
        return false;
    }
    if (send->args.size() != 0) {
        return false;
    }

    // self.sig
    if (ast::isa_tree<ast::Self>(send->recv.get())) {
        return true;
    }

    // Sorbet.sig
    auto recv = ast::cast_tree<ast::ConstantLit>(send->recv.get());
    if (recv && recv->symbol == core::Symbols::Sorbet()) {
        return true;
    }

    return false;
}

ParsedSig TypeSyntax::parseSig(core::MutableContext ctx, ast::Send *sigSend, const ParsedSig *parent,
                               bool allowSelfType, core::SymbolRef untypedBlame) {
    ParsedSig sig;

    ast::Send *send;

    if (isTProc(ctx, sigSend)) {
        send = sigSend;
    } else {
        sig.seen.sig = true;
        ENFORCE(sigSend->fun == core::Names::sig());
        auto block = ast::cast_tree<ast::Block>(sigSend->block.get());
        ENFORCE(block);
        send = ast::cast_tree<ast::Send>(block->body.get());
        if (!send) {
            // The resolver will error out, I guess give them back an empty sig?
            return sig;
        }
    }

    {
        ast::Send *tsend = send;
        // extract type parameters early
        while (tsend != nullptr) {
            if (tsend->fun == core::Names::typeParameters()) {
                if (parent != nullptr) {
                    if (auto e = ctx.state.beginError(tsend->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed signature; Type parameters can only be specified in outer sig");
                    }
                    break;
                }
                for (auto &arg : tsend->args) {
                    if (auto c = ast::cast_tree<ast::Literal>(arg.get())) {
                        if (c->isSymbol(ctx)) {
                            auto name = c->asSymbol(ctx);
                            auto &typeArgSpec = sig.enterTypeArgByName(name);
                            if (typeArgSpec.type) {
                                if (auto e = ctx.state.beginError(arg->loc,
                                                                  core::errors::Resolver::InvalidMethodSignature)) {
                                    e.setHeader("Malformed signature; Type argument `{}` was specified twice",
                                                name.toString(ctx));
                                }
                            }
                            typeArgSpec.type = make_shared<core::TypeVar>(core::Symbols::todo());
                            typeArgSpec.loc = arg->loc;
                        } else {
                            if (auto e =
                                    ctx.state.beginError(arg->loc, core::errors::Resolver::InvalidMethodSignature)) {
                                e.setHeader("Malformed signature; Type parameters are specified with symbols");
                            }
                        }
                    } else {
                        if (auto e = ctx.state.beginError(arg->loc, core::errors::Resolver::InvalidMethodSignature)) {
                            e.setHeader("Malformed signature; Type parameters are specified with symbols");
                        }
                    }
                }
            }
            tsend = ast::cast_tree<ast::Send>(tsend->recv.get());
        }
    }
    if (parent == nullptr) {
        parent = &sig;
    }

    while (send != nullptr) {
        switch (send->fun._id) {
            case core::Names::proc()._id:
                sig.seen.proc = true;
                break;
            case core::Names::params()._id: {
                if (sig.seen.params) {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: Multiple calls to `.params`", send->fun.toString(ctx));
                    }
                    sig.argTypes.clear();
                }
                sig.seen.params = true;

                if (send->args.empty()) {
                    break;
                }

                if (send->args.size() > 1) {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", send->fun.toString(ctx),
                                    "0-1", send->args.size());
                    }
                }
                auto *hash = ast::cast_tree<ast::Hash>(send->args[0].get());
                if (hash == nullptr) {
                    // Error will be reported in infer
                    break;
                }

                int i = 0;
                for (auto &key : hash->keys) {
                    auto &value = hash->values[i++];
                    auto lit = ast::cast_tree<ast::Literal>(key.get());
                    if (lit && lit->isSymbol(ctx)) {
                        core::NameRef name = lit->asSymbol(ctx);
                        sig.argTypes.emplace_back(ParsedSig::ArgSpec{
                            key->loc, name, getResultType(ctx, value, *parent, allowSelfType, untypedBlame)});
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
            case core::Names::override_()._id:
                sig.seen.override_ = true;
                break;
            case core::Names::implementation()._id:
                sig.seen.implementation = true;
                break;
            case core::Names::overridable()._id:
                sig.seen.overridable = true;
                break;
            case core::Names::returns()._id: {
                sig.seen.returns = true;
                if (send->args.size() != 1) {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", 1, "returns",
                                    send->args.size());
                    }
                    break;
                }

                auto nil = ast::cast_tree<ast::Literal>(send->args[0].get());
                if (nil && nil->isNil(ctx)) {
                    if (auto e =
                            ctx.state.beginError(send->args[0]->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("You probably meant .returns(NilClass)");
                    }
                    sig.returns = core::Types::nilClass();
                    break;
                }

                sig.returns = getResultType(ctx, send->args.front(), *parent, allowSelfType, untypedBlame);

                break;
            }
            case core::Names::void_()._id:
                sig.seen.void_ = true;
                sig.returns = core::Types::void_();
                break;
            case core::Names::checked()._id:
                sig.seen.checked = true;
                break;
            case core::Names::soft()._id:
                break;
            default:
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Method `{}` does not exist on `Sorbet::Private::Builder`", send->fun.toString(ctx));
                }
        }
        auto recv = ast::cast_tree<ast::Send>(send->recv.get());

        if (!recv) {
            auto self = ast::cast_tree<ast::Self>(send->recv.get());
            if (self) {
                self->claz = core::Symbols::Sorbet_Private_Builder();
            } else {
                ENFORCE(sig.seen.proc);
            }
            break;
        }

        send = recv;
    }
    ENFORCE(sig.seen.sig || sig.seen.proc);

    return sig;
}

shared_ptr<core::Type> interpretTCombinator(core::MutableContext ctx, ast::Send *send, const ParsedSig &sig,
                                            bool allowSelfType, core::SymbolRef untypedBlame) {
    switch (send->fun._id) {
        case core::Names::nilable()._id:
            if (send->args.size() != 1) {
                return core::Types::untypedUntracked(); // error will be reported in infer.
            }
            return core::Types::any(ctx,
                                    TypeSyntax::getResultType(ctx, send->args[0], sig, allowSelfType, untypedBlame),
                                    core::Types::nilClass());
        case core::Names::all()._id: {
            if (send->args.empty()) {
                // Error will be reported in infer
                return core::Types::untypedUntracked();
            }
            auto result = TypeSyntax::getResultType(ctx, send->args[0], sig, allowSelfType, untypedBlame);
            int i = 1;
            while (i < send->args.size()) {
                result = core::Types::all(
                    ctx, result, TypeSyntax::getResultType(ctx, send->args[i], sig, allowSelfType, untypedBlame));
                i++;
            }
            return result;
        }
        case core::Names::any()._id: {
            if (send->args.empty()) {
                // Error will be reported in infer
                return core::Types::untypedUntracked();
            }
            auto result = TypeSyntax::getResultType(ctx, send->args[0], sig, allowSelfType, untypedBlame);
            int i = 1;
            while (i < send->args.size()) {
                result = core::Types::any(
                    ctx, result, TypeSyntax::getResultType(ctx, send->args[i], sig, allowSelfType, untypedBlame));
                i++;
            }
            return result;
        }
        case core::Names::typeParameter()._id: {
            if (send->args.size() != 1) {
                // Error will be reported in infer
                return core::Types::untypedUntracked();
            }
            auto arr = ast::cast_tree<ast::Literal>(send->args[0].get());
            if (!arr || !arr->isSymbol(ctx)) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("type_parameter requires a symbol");
                }
                return core::Types::untypedUntracked();
            }
            auto fnd = sig.findTypeArgByName(arr->asSymbol(ctx));
            if (!fnd.type) {
                if (auto e = ctx.state.beginError(arr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
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
            auto arr = ast::cast_tree<ast::Array>(send->args[0].get());
            if (arr == nullptr) {
                // TODO(pay-server) unsilence this error and support enums from pay-server
                { return core::Types::Object(); }
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("enum must be passed a literal array. e.g. enum([1,\"foo\",MyClass])");
                }
                return core::Types::untypedUntracked();
            }
            if (arr->elems.empty()) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
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

            auto arg = send->args[0].get();

            auto *obj = ast::cast_tree<ast::ConstantLit>(arg);
            if (!obj) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.class_of needs a Class as its argument");
                }
                return core::Types::untypedUntracked();
            }
            auto sym = obj->symbol.data(ctx).dealias(ctx);
            auto singleton = sym.data(ctx).singletonClass(ctx);
            if (!singleton.exists()) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Unknown class");
                }
                return core::Types::untypedUntracked();
            }
            return make_shared<core::ClassType>(singleton);
        }
        case core::Names::untyped()._id:
            return core::Types::untyped(ctx, untypedBlame);
        case core::Names::selfType()._id:
            if (allowSelfType) {
                return make_shared<core::SelfType>();
            }
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Only top-level T.self_type is supported");
            }
            return core::Types::untypedUntracked();
        case core::Names::noreturn()._id:
            return core::Types::bottom();

        default:
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported method `{}`", "T." + send->fun.toString(ctx));
            }
            return core::Types::untypedUntracked();
    }
}

shared_ptr<core::Type> TypeSyntax::getResultType(core::MutableContext ctx, unique_ptr<ast::Expression> &expr,
                                                 const ParsedSig &sigBeingParsed, bool allowSelfType,
                                                 core::SymbolRef untypedBlame) {
    shared_ptr<core::Type> result;
    typecase(
        expr.get(),
        [&](ast::Array *arr) {
            vector<shared_ptr<core::Type>> elems;
            for (auto &el : arr->elems) {
                elems.emplace_back(getResultType(ctx, el, sigBeingParsed, false, untypedBlame));
            }
            result = core::TupleType::build(ctx, elems);
        },
        [&](ast::Hash *hash) {
            vector<shared_ptr<core::LiteralType>> keys;
            vector<shared_ptr<core::Type>> values;

            for (auto &ktree : hash->keys) {
                auto &vtree = hash->values[&ktree - &hash->keys.front()];
                auto val = getResultType(ctx, vtree, sigBeingParsed, false, untypedBlame);
                auto lit = ast::cast_tree<ast::Literal>(ktree.get());
                if (lit && (lit->isSymbol(ctx) || lit->isString(ctx))) {
                    auto keytype = dynamic_pointer_cast<core::LiteralType>(lit->value);
                    ENFORCE(keytype);
                    keys.emplace_back(keytype);
                    values.emplace_back(val);
                } else {
                    if (auto e = ctx.state.beginError(ktree->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed type declaration. Shape keys must be literals");
                    }
                }
            }
            result = make_shared<core::ShapeType>(core::Types::hashOfUntyped(), keys, values);
        },
        [&](ast::ConstantLit *i) {
            if (i->typeAlias) {
                result = getResultType(ctx, i->typeAlias, sigBeingParsed, allowSelfType, untypedBlame);
                return;
            }
            bool silenceGenericError = i->symbol == core::Symbols::Hash() || i->symbol == core::Symbols::Array() ||
                                       i->symbol == core::Symbols::Set() || i->symbol == core::Symbols::Struct() ||
                                       i->symbol == core::Symbols::File();
            // TODO: reduce this^^^ set.
            auto sym = i->symbol.data(ctx).dealias(ctx);
            if (sym.data(ctx).isClass()) {
                if (sym.data(ctx).typeArity(ctx) > 0 && !silenceGenericError) {
                    if (auto e = ctx.state.beginError(i->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed type declaration. Generic class without type arguments `{}`",
                                    i->symbol.show(ctx));
                    }
                }
                result = sym.data(ctx).externalType(ctx);
            } else if (sym.data(ctx).isTypeMember()) {
                result = make_shared<core::LambdaParam>(sym);
            } else if (sym.data(ctx).isStaticField()) {
                if (auto e = ctx.state.beginError(i->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Constant `{}` is not a class or type alias", i->symbol.show(ctx));
                    e.addErrorLine(sym.data(ctx).loc(),
                                   "If you are trying to define a type alias, you should use `{}` here",
                                   "T.type_alias");
                }
                result = core::Types::untypedUntracked();
            } else {
                if (auto e = ctx.state.beginError(i->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Not a class type `{}`", i->symbol.show(ctx));
                }
                result = core::Types::untypedUntracked();
            }
        },
        [&](ast::Send *s) {
            if (isTProc(ctx, s)) {
                auto sig = parseSig(ctx, s, &sigBeingParsed, false, untypedBlame);

                vector<shared_ptr<core::Type>> targs;

                if (sig.returns == nullptr) {
                    if (auto e = ctx.state.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed T.proc: You must specify a return type");
                    }
                    targs.emplace_back(core::Types::untypedUntracked());
                } else {
                    targs.emplace_back(sig.returns);
                }

                for (auto &arg : sig.argTypes) {
                    targs.push_back(arg.type);
                }

                auto arity = targs.size() - 1;
                if (arity > core::Symbols::MAX_PROC_ARITY) {
                    if (auto e = ctx.state.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed T.proc: Too many arguments (max `{}`)", core::Symbols::MAX_PROC_ARITY);
                    }
                    result = core::Types::untypedUntracked();
                    return;
                }
                auto sym = core::Symbols::Proc(arity);

                result = make_shared<core::AppliedType>(sym, targs);
                return;
            }
            auto recv = s->recv.get();

            auto *recvi = ast::cast_tree<ast::ConstantLit>(recv);
            if (recvi == nullptr) {
                if (auto e = ctx.state.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
                }
                result = core::Types::untypedUntracked();
                return;
            }
            if (recvi->symbol == core::Symbols::T()) {
                result = interpretTCombinator(ctx, s, sigBeingParsed, allowSelfType, untypedBlame);
                return;
            }

            if (recvi->symbol == core::Symbols::Magic() && s->fun == core::Names::callWithSplat()) {
                // TODO(pay-server) remove this block
                if (auto e = ctx.state.beginError(recvi->loc, core::errors::Resolver::InvalidTypeDeclarationTyped)) {
                    e.setHeader("Splats are unsupported by the static checker and banned in typed code");
                }
                result = core::Types::untypedUntracked();
                return;
            }

            if (s->fun != core::Names::squareBrackets()) {
                if (auto e = ctx.state.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
                }
            }

            InlinedVector<unique_ptr<core::TypeAndOrigins>, 2> holders;
            InlinedVector<const core::TypeAndOrigins *, 2> targs;
            InlinedVector<core::Loc, 2> argLocs;
            targs.reserve(s->args.size());
            argLocs.reserve(s->args.size());
            holders.reserve(s->args.size());
            for (auto &arg : s->args) {
                core::TypeAndOrigins ty;
                ty.origins.emplace_back(arg->loc);
                ty.type = make_shared<core::MetaType>(
                    TypeSyntax::getResultType(ctx, arg, sigBeingParsed, false, untypedBlame));
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
            } else if (recvi->symbol == core::Symbols::Range()) {
                corrected = core::Symbols::T_Range();
            } else if (recvi->symbol == core::Symbols::Set()) {
                corrected = core::Symbols::T_Set();
            }
            if (corrected.exists()) {
                if (auto e = ctx.state.beginError(s->loc, core::errors::Resolver::BadStdlibGeneric)) {
                    e.setHeader("Use `{}`, not `{}` to declare a typed `{}`", corrected.data(ctx).show(ctx) + "[...]",
                                recvi->symbol.data(ctx).show(ctx) + "[...]", recvi->symbol.data(ctx).show(ctx));
                    e.addErrorSection(
                        core::ErrorSection(core::ErrorColors::format("`{}` will not work in the runtime type system.",
                                                                     recvi->symbol.data(ctx).show(ctx) + "[...]")));
                }
            } else {
                corrected = recvi->symbol;
            }

            auto ctype = make_shared<core::ClassType>(corrected.data(ctx).singletonClass(ctx));
            core::CallLocs locs{
                s->loc,
                recvi->loc,
                argLocs,
            };
            core::DispatchArgs dispatchArgs{core::Names::squareBrackets(), locs, targs, ctype, ctype, nullptr};
            auto dispatched = ctype->dispatchCall(ctx, dispatchArgs);
            for (auto &comp : dispatched.components) {
                for (auto &err : comp.errors) {
                    ctx.state._error(move(err));
                }
            }
            auto out = dispatched.returnType;

            if (out->isUntyped()) {
                result = out;
                return;
            }
            if (auto *mt = core::cast_type<core::MetaType>(out.get())) {
                result = mt->wrapped;
                return;
            }

            if (auto e = ctx.state.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
            }
            result = core::Types::untypedUntracked();
        },
        [&](ast::Self *slf) {
            core::SymbolRef klass = ctx.owner.data(ctx).enclosingClass(ctx);
            result = klass.data(ctx).selfType(ctx);
        },
        [&](ast::Expression *expr) {
            if (auto e = ctx.state.beginError(expr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported type syntax");
            }
            result = core::Types::untypedUntracked();
        });
    ENFORCE(result.get() != nullptr);
    result->sanityCheck(ctx);
    return result;
}

ParsedSig::TypeArgSpec &ParsedSig::enterTypeArgByName(core::NameRef name) {
    for (auto &current : typeArgs) {
        if (current.name == name) {
            return current;
        }
    }
    typeArgs.emplace_back();
    typeArgs.back().name = name;
    return typeArgs.back();
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
} // namespace resolver
} // namespace sorbet
