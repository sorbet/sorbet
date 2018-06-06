#include "resolver/type_syntax.h"
#include "core/Names/resolver.h"
#include "core/core.h"
#include "core/errors/resolver.h"

using namespace std;

namespace ruby_typer {
namespace resolver {

shared_ptr<core::Type> getResultLiteral(core::MutableContext ctx, unique_ptr<ast::Expression> &expr) {
    shared_ptr<core::Type> result;
    typecase(expr.get(), [&](ast::Literal *lit) { result = lit->value; },
             [&](ast::Expression *expr) {
                 if (auto e = ctx.state.beginError(expr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                     e.setHeader("Unsupported type literal");
                 }
                 result = core::Types::dynamic();
             });
    ENFORCE(result.get() != nullptr);
    result->sanityCheck(ctx);
    return result;
}

bool isTProc(core::MutableContext ctx, ast::Send *send) {
    while (send != nullptr) {
        if (send->fun == core::Names::proc()) {
            if (auto *rcv = ast::cast_tree<ast::Ident>(send->recv.get())) {
                if (rcv->symbol == core::Symbols::T()) {
                    return true;
                }
            }
        }
        send = ast::cast_tree<ast::Send>(send->recv.get());
    }
    return false;
}

bool TypeSyntax::isSig(core::MutableContext ctx, ast::Send *send) {
    bool sawSig = false;
    while (send != nullptr) {
        sawSig = sawSig || send->fun == core::Names::sig();
        if ((send->fun == core::Names::sig() || send->fun == core::Names::typeParameters()) && sawSig &&
            ast::cast_tree<ast::Self>(send->recv.get()) != nullptr) {
            return true;
        }
        send = ast::cast_tree<ast::Send>(send->recv.get());
    }
    return false;
}

ParsedSig TypeSyntax::parseSig(core::MutableContext ctx, ast::Send *send, const ParsedSig *parent) {
    ParsedSig sig;

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
                            typeArgSpec.type = std::make_shared<core::TypeVar>(core::Symbols::todo());
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
            case core::Names::sig()._id:
            case core::Names::proc()._id: {
                if (sig.seen.sig || sig.seen.proc) {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`: Found multiple argument lists", send->fun.toString(ctx));
                    }
                    sig.argTypes.clear();
                }
                if (send->fun == core::Names::sig()) {
                    sig.seen.sig = true;
                } else {
                    sig.seen.proc = true;
                }

                if (send->args.empty()) {
                    break;
                }
                sig.seen.args = true;

                if (send->args.size() > 1) {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", send->fun.toString(ctx),
                                    "0-1", send->args.size());
                    }
                }
                auto *hash = ast::cast_tree<ast::Hash>(send->args[0].get());
                if (hash == nullptr) {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`; Expected a hash of arguments => types", send->fun.toString(ctx),
                                    send->args.size());
                    }
                    break;
                }

                int i = 0;
                for (auto &key : hash->keys) {
                    auto &value = hash->values[i++];
                    auto lit = ast::cast_tree<ast::Literal>(key.get());
                    if (lit && lit->isSymbol(ctx)) {
                        core::NameRef name = lit->asSymbol(ctx);
                        sig.argTypes.emplace_back(
                            ParsedSig::ArgSpec{key->loc, name, getResultType(ctx, value, *parent)});
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
                        e.setHeader("Wrong number of args to `{}`. Expected: `{}`, got: `{}`", 1, "sig.returns",
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

                sig.returns = getResultType(ctx, send->args.front(), *parent);

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
                    e.setHeader("Unknown `sig` builder method `{}`", send->fun.toString(ctx));
                }
        }
        send = ast::cast_tree<ast::Send>(send->recv.get());
    }
    ENFORCE(sig.seen.sig || sig.seen.proc);

    return sig;
}

shared_ptr<core::Type> interpretTCombinator(core::MutableContext ctx, ast::Send *send, const ParsedSig &sig) {
    switch (send->fun._id) {
        case core::Names::nilable()._id:
            return core::Types::any(ctx, TypeSyntax::getResultType(ctx, send->args[0], sig), core::Types::nilClass());
        case core::Names::all()._id: {
            if (send->args.empty()) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.all() needs one or more type arguments");
                }
                return core::Types::dynamic();
            }
            auto result = TypeSyntax::getResultType(ctx, send->args[0], sig);
            int i = 1;
            while (i < send->args.size()) {
                result = core::Types::all(ctx, result, TypeSyntax::getResultType(ctx, send->args[i], sig));
                i++;
            }
            return result;
        }
        case core::Names::any()._id: {
            if (send->args.empty()) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.all() needs one or more type arguments");
                    e.addErrorSection(
                        core::ErrorSection("Hint: if you want to allow any type as an argument, use `T.untyped`"));
                }
                return core::Types::dynamic();
            }
            auto result = TypeSyntax::getResultType(ctx, send->args[0], sig);
            int i = 1;
            while (i < send->args.size()) {
                result = core::Types::any(ctx, result, TypeSyntax::getResultType(ctx, send->args[i], sig));
                i++;
            }
            return result;
        }
        case core::Names::typeParameter()._id: {
            if (send->args.size() != 1) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("type_parameter only takes a single argument");
                }
                return core::Types::dynamic();
            }
            auto arr = ast::cast_tree<ast::Literal>(send->args[0].get());
            if (!arr || !arr->isSymbol(ctx)) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("type_parameter requires a symbol");
                }
                return core::Types::dynamic();
            }
            auto fnd = sig.findTypeArgByName(arr->asSymbol(ctx));
            if (!fnd.type) {
                if (auto e = ctx.state.beginError(arr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Unspecified type parameter");
                }
                return core::Types::dynamic();
            }
            return fnd.type;
        }
        case core::Names::enum_()._id: {
            if (send->args.size() != 1) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("enum only takes a single argument");
                }
                return core::Types::dynamic();
            }
            auto arr = ast::cast_tree<ast::Array>(send->args[0].get());
            if (arr == nullptr) {
                // TODO(pay-server) unsilence this error and support enums from pay-server
                { return core::Types::Object(); }
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("enum must be passed a literal array. e.g. enum([1,\"foo\",MyClass])");
                }
                return core::Types::dynamic();
            }
            if (arr->elems.empty()) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("enum([]) is invalid");
                }
                return core::Types::dynamic();
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
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.class_of only takes a single argument");
                }
                return core::Types::dynamic();
            }
            auto *obj = ast::cast_tree<ast::Ident>(send->args[0].get());
            if (!obj) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("T.class_of needs a Class as its argument");
                }
                return core::Types::dynamic();
            }
            auto sym = obj->symbol.data(ctx).dealias(ctx);
            auto singleton = sym.data(ctx).singletonClass(ctx);
            if (!singleton.exists()) {
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Unknown class");
                }
                return core::Types::dynamic();
            }
            return make_shared<core::ClassType>(singleton);
        }
        case core::Names::untyped()._id:
            return core::Types::dynamic();

        case core::Names::noreturn()._id:
            return core::Types::bottom();

        default:
            if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported method `{}`", "T." + send->fun.toString(ctx));
            }
            return core::Types::dynamic();
    }
}

shared_ptr<core::Type> TypeSyntax::getResultType(core::MutableContext ctx, unique_ptr<ast::Expression> &expr,
                                                 const ParsedSig &sigBeingParsed) {
    shared_ptr<core::Type> result;
    typecase(
        expr.get(),
        [&](ast::Array *arr) {
            vector<shared_ptr<core::Type>> elems;
            for (auto &el : arr->elems) {
                elems.emplace_back(getResultType(ctx, el, sigBeingParsed));
            }
            result = core::TupleType::build(ctx, elems);
        },
        [&](ast::Hash *hash) {
            vector<shared_ptr<core::LiteralType>> keys;
            vector<shared_ptr<core::Type>> values;

            for (auto &ktree : hash->keys) {
                auto &vtree = hash->values[&ktree - &hash->keys.front()];
                auto val = getResultType(ctx, vtree, sigBeingParsed);
                auto lit = ast::cast_tree<ast::Literal>(ktree.get());
                if (lit && (lit->isSymbol(ctx) || lit->isString(ctx))) {
                    auto keytype = std::dynamic_pointer_cast<core::LiteralType>(lit->value);
                    ENFORCE(keytype);
                    keys.emplace_back(keytype);
                    values.emplace_back(val);
                } else {
                    if (auto e = ctx.state.beginError(ktree->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed type declaration. Shape keys must be literals");
                    }
                }
            }
            result = make_shared<core::ShapeType>(keys, values);
        },
        [&](ast::Ident *i) {
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
            } else {
                if (auto e = ctx.state.beginError(i->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Not a class type `{}`", i->symbol.show(ctx));
                }
                result = core::Types::dynamic();
            }
        },
        [&](ast::Send *s) {
            if (isTProc(ctx, s)) {
                auto sig = parseSig(ctx, s, &sigBeingParsed);

                vector<shared_ptr<core::Type>> targs;

                if (sig.returns == nullptr) {
                    if (auto e = ctx.state.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed T.proc: You must specify a return type");
                    }
                    targs.emplace_back(core::Types::dynamic());
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
                    result = core::Types::dynamic();
                    return;
                }
                auto sym = core::Symbols::Proc(arity);

                result = make_shared<core::AppliedType>(sym, targs);
                return;
            }

            auto *recvi = ast::cast_tree<ast::Ident>(s->recv.get());
            if (recvi == nullptr) {
                if (auto e = ctx.state.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
                }
                result = core::Types::dynamic();
                return;
            }
            if (recvi->symbol == core::Symbols::T()) {
                result = interpretTCombinator(ctx, s, sigBeingParsed);
                return;
            }

            if (recvi->symbol == core::Symbols::Magic() && s->fun == core::Names::splat()) {
                // TODO(pay-server) remove this block
                if (auto e = ctx.state.beginError(recvi->loc, core::errors::Resolver::InvalidTypeDeclarationTyped)) {
                    e.setHeader("Splats are unsupported by the static checker and banned in typed code");
                }
                result = core::Types::dynamic();
                return;
            }

            if (s->fun == core::Names::singletonClass()) {
                auto sym = recvi->symbol.data(ctx).dealias(ctx);
                auto singleton = sym.data(ctx).singletonClass(ctx);
                if (singleton.exists()) {
                    result = make_shared<core::ClassType>(singleton);
                    return;
                }
            }

            if (s->fun != core::Names::squareBrackets()) {
                if (auto e = ctx.state.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Unknown type syntax. Expected a ClassName or T.<func>");
                }
            }

            std::vector<core::TypeAndOrigins> targs;
            for (auto &arg : s->args) {
                core::TypeAndOrigins ty;
                ty.origins.emplace_back(arg->loc);
                ty.type = make_shared<core::MetaType>(TypeSyntax::getResultType(ctx, arg, sigBeingParsed));
                targs.emplace_back(ty);
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
            auto out = ctype->dispatchCall(ctx, core::Names::squareBrackets(), s->loc, targs, ctype, ctype, nullptr);

            if (out->isDynamic()) {
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
            result = core::Types::dynamic();
        },
        [&](ast::Self *slf) {
            core::SymbolRef klass = ctx.owner.data(ctx).enclosingClass(ctx);
            result = klass.data(ctx).selfType(ctx);
        },
        [&](ast::Expression *expr) {
            if (auto e = ctx.state.beginError(expr->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                e.setHeader("Unsupported type syntax");
            }
            result = core::Types::dynamic();
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
} // namespace ruby_typer
