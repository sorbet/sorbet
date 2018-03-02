#include "resolver/type_syntax.h"
#include "core/Names/resolver.h"
#include "core/core.h"
#include "core/errors/resolver.h"

using namespace std;

namespace ruby_typer {
namespace resolver {

shared_ptr<core::Type> getResultLiteral(core::MutableContext ctx, unique_ptr<ast::Expression> &expr) {
    shared_ptr<core::Type> result;
    typecase(expr.get(), [&](ast::IntLit *lit) { result = make_shared<core::LiteralType>(lit->value); },
             [&](ast::FloatLit *lit) { result = make_shared<core::LiteralType>(lit->value); },
             [&](ast::BoolLit *lit) { result = make_shared<core::LiteralType>(lit->value); },
             [&](ast::StringLit *lit) { result = make_shared<core::LiteralType>(core::Symbols::String(), lit->value); },
             [&](ast::SymbolLit *lit) { result = make_shared<core::LiteralType>(core::Symbols::Symbol(), lit->name); },
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
    while (send != nullptr) {
        if (send->fun == core::Names::sig() && ast::cast_tree<ast::Self>(send->recv.get()) != nullptr) {
            return true;
        }
        send = ast::cast_tree<ast::Send>(send->recv.get());
    }
    return false;
}

ParsedSig TypeSyntax::parseSig(core::MutableContext ctx, ast::Send *send) {
    ParsedSig sig;

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
                        e.setHeader("Wrong number of args to `{}`. Got {}, expected 0-1", send->fun.toString(ctx),
                                    send->args.size());
                    }
                }
                auto *hash = ast::cast_tree<ast::Hash>(send->args[0].get());
                if (hash == nullptr) {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Malformed `{}`; Expected a hash of arguments => types.", send->fun.toString(ctx),
                                    send->args.size());
                    }
                    break;
                }

                int i = 0;
                for (auto &key : hash->keys) {
                    auto &value = hash->values[i++];
                    if (auto *symbolLit = ast::cast_tree<ast::SymbolLit>(key.get())) {
                        sig.argTypes.emplace_back(
                            ParsedSig::ArgSpec{key->loc, symbolLit->name, getResultType(ctx, value)});
                    }
                }
                break;
            }
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
            case core::Names::returns()._id:
                sig.seen.returns = true;
                if (send->args.size() != 1) {
                    if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                        e.setHeader("Wrong number of args to `sig.returns`. Got {}, expected 1", send->args.size());
                    }
                }
                if (!send->args.empty()) {
                    sig.returns = getResultType(ctx, send->args.front());
                }

                break;
            case core::Names::checked()._id:
                sig.seen.checked = true;
                break;
            default:
                if (auto e = ctx.state.beginError(send->loc, core::errors::Resolver::InvalidMethodSignature)) {
                    e.setHeader("Unknown `sig` builder method {}.", send->fun.toString(ctx));
                }
        }
        send = ast::cast_tree<ast::Send>(send->recv.get());
    }
    ENFORCE(sig.seen.sig || sig.seen.proc);

    return sig;
}

shared_ptr<core::Type> interpretTCombinator(core::MutableContext ctx, ast::Send *send) {
    switch (send->fun._id) {
        case core::Names::nilable()._id:
            return core::Types::buildOr(ctx, TypeSyntax::getResultType(ctx, send->args[0]), core::Types::nil());
        case core::Names::all()._id: {
            auto result = TypeSyntax::getResultType(ctx, send->args[0]);
            int i = 1;
            while (i < send->args.size()) {
                result = core::Types::buildAnd(ctx, result, TypeSyntax::getResultType(ctx, send->args[i]));
                i++;
            }
            return result;
        }
        case core::Names::any()._id: {
            auto result = TypeSyntax::getResultType(ctx, send->args[0]);
            int i = 1;
            while (i < send->args.size()) {
                result = core::Types::buildOr(ctx, result, TypeSyntax::getResultType(ctx, send->args[i]));
                i++;
            }
            return result;
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
                { return core::Types::bottom(); }
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
                result = core::Types::buildOr(ctx, result, getResultLiteral(ctx, arr->elems[i]));
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
                e.setHeader("Unsupported method T.{}", send->fun.toString(ctx));
            }
            return core::Types::dynamic();
    }
}

shared_ptr<core::Type> TypeSyntax::getResultType(core::MutableContext ctx, unique_ptr<ast::Expression> &expr) {
    shared_ptr<core::Type> result;
    typecase(
        expr.get(),
        [&](ast::Array *arr) {
            vector<shared_ptr<core::Type>> elems;
            for (auto &el : arr->elems) {
                elems.emplace_back(getResultType(ctx, el));
            }
            result = make_shared<core::TupleType>(elems);
        },
        [&](ast::Hash *hash) {
            vector<shared_ptr<core::LiteralType>> keys;
            vector<shared_ptr<core::Type>> values;

            for (auto &ktree : hash->keys) {
                auto &vtree = hash->values[&ktree - &hash->keys.front()];
                auto val = getResultType(ctx, vtree);
                if (auto *sym = ast::cast_tree<ast::SymbolLit>(ktree.get())) {
                    auto key = make_shared<core::LiteralType>(core::Symbols::Symbol(), sym->name);
                    keys.emplace_back(key);
                    values.emplace_back(val);
                } else {
                    if (auto e = ctx.state.beginError(ktree->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed type declaration. Shape keys must be Symbol literals.");
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
                        e.setHeader("Malformed type declaration. Generic class without type arguments {}",
                                    i->toString(ctx));
                    }
                }
                result = sym.data(ctx).externalType(ctx);
            } else if (sym.data(ctx).isTypeMember()) {
                result = make_shared<core::LambdaParam>(sym);
            } else {
                if (auto e = ctx.state.beginError(i->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                    e.setHeader("Malformed type declaration. Not a class type {}", i->toString(ctx));
                }
                result = core::Types::dynamic();
            }
        },
        [&](ast::Send *s) {
            if (isTProc(ctx, s)) {
                auto sig = parseSig(ctx, s);

                vector<shared_ptr<core::Type>> targs;

                if (sig.returns == nullptr) {
                    if (auto e = ctx.state.beginError(s->loc, core::errors::Resolver::InvalidTypeDeclaration)) {
                        e.setHeader("Malformed T.proc: You must specify a return type.");
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
                        e.setHeader("Malformed T.proc: Too many arguments (max {})", core::Symbols::MAX_PROC_ARITY);
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
                    e.setHeader("Malformed type declaration. Unknown type syntax {}", expr->toString(ctx));
                }
                result = core::Types::dynamic();
                return;
            }
            if (recvi->symbol == core::Symbols::T()) {
                result = interpretTCombinator(ctx, s);
                return;
            }

            if (recvi->symbol == core::Symbols::Magic() && s->fun == core::Names::splat()) {
                // TODO(pay-server) remove this block
                result = core::Types::bottom();
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
                    e.setHeader("Malformed type declaration. Unknown type syntax {}", expr->toString(ctx));
                }
            }

            std::vector<core::TypeAndOrigins> targs;
            for (auto &arg : s->args) {
                core::TypeAndOrigins ty;
                ty.origins.emplace_back(arg->loc);
                ty.type = make_shared<core::MetaType>(TypeSyntax::getResultType(ctx, arg));
                targs.emplace_back(ty);
            }
            auto ctype = make_shared<core::ClassType>(recvi->symbol.data(ctx).singletonClass(ctx));
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
                e.setHeader("Illegal type expression. Unknown type {}", out->toString(ctx));
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

} // namespace resolver
} // namespace ruby_typer
