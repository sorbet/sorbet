#include "core/errors/resolver.h"
#include "../ast/Trees.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/Names/resolver.h"
#include "core/core.h"
#include "resolver/resolver.h"

#include <algorithm> // find_if
#include <list>
#include <vector>

using namespace std;

namespace ruby_typer {
namespace resolver {

struct Nesting {
    unique_ptr<Nesting> parent;
    core::SymbolRef scope;

    Nesting(unique_ptr<Nesting> parent, core::SymbolRef scope) : parent(move(parent)), scope(scope) {}

    Nesting(const Nesting &rhs) = delete;
    Nesting(Nesting &&rhs) = delete;
};

class ResolveConstantsWalk {
private:
    core::SymbolRef resolveLhs(core::Context ctx, core::NameRef name) {
        Nesting *scope = nesting_.get();
        while (scope != nullptr) {
            auto lookup = scope->scope.data(ctx).findMember(ctx, name);
            if (lookup.exists()) {
                return lookup;
            }
            scope = scope->parent.get();
        }
        return core::Symbols::noSymbol();
    }

    core::SymbolRef resolveConstant(core::Context ctx, ast::ConstantLit *c) {
        if (ast::isa_tree<ast::EmptyTree>(c->scope.get())) {
            core::SymbolRef result = resolveLhs(ctx, c->cnst);
            if (!result.exists()) {
                ctx.state.error(c->loc, core::errors::Resolver::StubConstant, "Stubbing out unknown constant {}",
                                c->toString(ctx));
                result = ctx.state.enterClassSymbol(c->loc, nesting_.get()->scope, c->cnst);
                result.data(ctx).resultType = core::Types::dynamic();
                result.data(ctx).setIsModule(false);
            }
            return result;

        } else if (ast::ConstantLit *scope = ast::cast_tree<ast::ConstantLit>(c->scope.get())) {
            auto resolved = resolveConstant(ctx, scope);
            if (!resolved.exists() || resolved == core::Symbols::untyped()) {
                return resolved;
            }
            c->scope = make_unique<ast::Ident>(c->loc, resolved);
        }

        if (ast::Ident *id = ast::cast_tree<ast::Ident>(c->scope.get())) {
            core::SymbolRef resolved = id->symbol;
            core::SymbolRef result = resolved.data(ctx).findMember(ctx, c->cnst);
            if (!result.exists()) {
                if (resolved.data(ctx).resultType.get() == nullptr || !resolved.data(ctx).resultType->isDynamic()) {
                    ctx.state.error(c->loc, core::errors::Resolver::StubConstant, "Stubbing out unknown constant {}",
                                    c->toString(ctx));
                }
                result = ctx.state.enterClassSymbol(c->loc, resolved, c->cnst);
                result.data(ctx).resultType = core::Types::dynamic();
                result.data(ctx).setIsModule(false);
            }

            return result;
        } else {
            ctx.state.error(c->loc, core::errors::Resolver::DynamicConstant,
                            "Dynamic constant references are unsupported {}", c->toString(ctx));
            return core::Symbols::untyped();
        }
    }

    unique_ptr<ast::Expression> maybeResolve(core::Context ctx, ast::Expression *expr) {
        if (ast::ConstantLit *cnst = ast::cast_tree<ast::ConstantLit>(expr)) {
            core::SymbolRef resolved = resolveConstant(ctx, cnst);
            if (resolved.exists()) {
                return make_unique<ast::Ident>(expr->loc, resolved);
            }
        }
        return nullptr;
    }

public:
    ResolveConstantsWalk(core::Context ctx) : nesting_(make_unique<Nesting>(nullptr, core::Symbols::root())) {}

    ast::ClassDef *preTransformClassDef(core::Context ctx, ast::ClassDef *original) {
        nesting_ = make_unique<Nesting>(move(nesting_), original->symbol);
        return original;
    }
    ast::Expression *postTransformClassDef(core::Context ctx, ast::ClassDef *original) {
        nesting_ = move(nesting_->parent);

        for (auto &ancst : original->ancestors) {
            if (auto resolved = maybeResolve(ctx, ancst.get())) {
                ancst.swap(resolved);
            }
        }
        core::Symbol &data = original->symbol.data(ctx);
        if (original->kind == ast::Module && data.mixins(ctx).empty()) {
            data.mixins(ctx).emplace_back(core::Symbols::BasicObject());
        }
        for (auto &ancst : original->ancestors) {
            ast::Ident *id = ast::cast_tree<ast::Ident>(ancst.get());
            if (id == nullptr || !id->symbol.data(ctx).isClass()) {
                ctx.state.error(ancst->loc, core::errors::Resolver::DynamicSuperclass,
                                "Superclasses and mixins must be statically resolved.");
                continue;
            }
            if (id->symbol == original->symbol || id->symbol.data(ctx).derivesFrom(ctx, original->symbol)) {
                ctx.state.error(id->loc, core::errors::Resolver::CircularDependency,
                                "Circular dependency: {} and {} are declared as parents of each other",
                                original->symbol.data(ctx).name.toString(ctx), id->symbol.data(ctx).name.toString(ctx));
                continue;
            }

            if (original->kind == ast::Class && &ancst == &original->ancestors.front()) {
                if (id->symbol == core::Symbols::todo()) {
                    continue;
                }
                if (!data.superClass.exists() || data.superClass == core::Symbols::todo() ||
                    data.superClass == id->symbol) {
                    data.superClass = id->symbol;
                } else {
                    ctx.state.error(id->loc, core::errors::Resolver::RedefinitionOfParents,
                                    "Class parents redefined for class {}",
                                    original->symbol.data(ctx).name.toString(ctx));
                }
            } else {
                data.argumentsOrMixins.emplace_back(id->symbol);
            }
        }

        return original;
    }

    ast::Expression *postTransformConstantLit(core::Context ctx, ast::ConstantLit *c) {
        core::SymbolRef resolved = resolveConstant(ctx, c);
        if (!resolved.exists()) {
            string str = c->toString(ctx);
            resolved = resolveConstant(ctx, c);
            return c;
        }
        return new ast::Ident(c->loc, resolved);
    }

    ast::Assign *postTransformAssign(core::Context ctx, ast::Assign *asgn) {
        auto *id = ast::cast_tree<ast::Ident>(asgn->lhs.get());
        if (id == nullptr || !id->symbol.data(ctx).isStaticField()) {
            return asgn;
        }

        auto *rhs = ast::cast_tree<ast::Ident>(asgn->rhs.get());
        if (rhs == nullptr || !rhs->symbol.data(ctx).isClass()) {
            return asgn;
        }

        id->symbol.data(ctx).resultType = make_unique<core::ClassType>(rhs->symbol.data(ctx).singletonClass(ctx));
        return asgn;
    }

    unique_ptr<Nesting> nesting_;
};

class ResolveSignaturesWalk {
private:
    void processDeclareVariables(core::Context ctx, ast::Send *send) {
        if (send->block != nullptr) {
            ctx.state.error(send->loc, core::errors::Resolver::InvalidDeclareVariables,
                            "Malformed `declare_variables'");
            return;
        }

        if (send->args.size() != 1) {
            ctx.state.error(send->loc, core::errors::Resolver::InvalidDeclareVariables,
                            "Wrong number of arguments to `declare_variables'");
            return;
        }
        auto hash = ast::cast_tree<ast::Hash>(send->args.front().get());
        if (hash == nullptr) {
            ctx.state.error(send->loc, core::errors::Resolver::InvalidDeclareVariables,
                            "Malformed `declare_variables': Argument must be a hash");
            return;
        }
        for (int i = 0; i < hash->keys.size(); ++i) {
            auto &key = hash->keys[i];
            auto &value = hash->values[i];
            auto sym = ast::cast_tree<ast::SymbolLit>(key.get());
            if (sym == nullptr) {
                ctx.state.error(key->loc, core::errors::Resolver::InvalidDeclareVariables,
                                "`declare_variables': variable names must be symbols");
                continue;
            }

            auto typ = getResultType(ctx, value);
            core::SymbolRef var;

            auto str = sym->name.toString(ctx);
            if (str.substr(0, 2) == "@@") {
                core::Symbol &data = ctx.owner.data(ctx);
                var = data.findMember(ctx, sym->name);
                if (var.exists()) {
                    ctx.state.error(key->loc, core::errors::Resolver::DuplicateVariableDeclaration,
                                    "Redeclaring variable `{}'", str);
                } else {
                    var = ctx.state.enterStaticFieldSymbol(sym->loc, ctx.owner, sym->name);
                }
            } else if (str.substr(0, 1) == "@") {
                core::Symbol &data = ctx.owner.data(ctx);
                var = data.findMember(ctx, sym->name);
                if (var.exists()) {
                    ctx.state.error(key->loc, core::errors::Resolver::DuplicateVariableDeclaration,
                                    "Redeclaring variable `{}'", str);
                } else {
                    var = ctx.state.enterFieldSymbol(sym->loc, ctx.owner, sym->name);
                }
            } else {
                ctx.state.error(key->loc, core::errors::Resolver::InvalidDeclareVariables,
                                "`declare_variables`: variables must start with @ or @@");
            }

            if (var.exists()) {
                var.data(ctx).resultType = typ;
            }
        }
    }

    void defineAttr(core::Context ctx, ast::Send *send, bool r, bool w) {
        for (auto &arg : send->args) {
            auto name = core::NameRef::noName();
            if (auto *sym = ast::cast_tree<ast::SymbolLit>(arg.get())) {
                name = sym->name;
            } else if (auto *str = ast::cast_tree<ast::StringLit>(arg.get())) {
                name = str->value;
            } else {
                ctx.state.error(arg->loc, core::errors::Resolver::InvalidAttr,
                                "{} argument must be a string or symbol literal", send->fun.toString(ctx));
                continue;
            }

            core::NameRef varName = ctx.state.enterNameUTF8("@" + name.toString(ctx));
            core::SymbolRef sym = ctx.owner.data(ctx).findMemberTransitive(ctx, varName);
            if (!sym.exists()) {
                ctx.state.error(arg->loc, core::errors::Resolver::UndeclaredVariable,
                                "Accessor for undeclared variable `{}'", varName.toString(ctx));
                sym = ctx.state.enterFieldSymbol(arg->loc, ctx.owner, varName);
            }

            if (r) {
                core::SymbolRef meth = ctx.state.enterMethodSymbol(send->loc, ctx.owner, name);
                meth.data(ctx).resultType = sym.data(ctx).resultType;
            }
            if (w) {
                core::SymbolRef meth = ctx.state.enterMethodSymbol(send->loc, ctx.owner, name.addEq(ctx));
                core::SymbolRef arg0 = ctx.state.enterMethodArgumentSymbol(arg->loc, meth, core::Names::arg0());

                auto ty = sym.data(ctx).resultType;
                if (ty == nullptr) {
                    ty = core::Types::dynamic();
                }
                arg0.data(ctx).resultType = ty;
                meth.data(ctx).arguments().push_back(arg0);
                meth.data(ctx).resultType = ty;
            }
        }
    }

    shared_ptr<core::Type> getResultLiteral(core::Context ctx, unique_ptr<ast::Expression> &expr) {
        shared_ptr<core::Type> result;
        typecase(
            expr.get(), [&](ast::IntLit *lit) { result = make_shared<core::LiteralType>(lit->value); },
            [&](ast::FloatLit *lit) { result = make_shared<core::LiteralType>(lit->value); },
            [&](ast::BoolLit *lit) { result = make_shared<core::LiteralType>(lit->value); },
            [&](ast::StringLit *lit) { result = make_shared<core::LiteralType>(core::Symbols::String(), lit->value); },
            [&](ast::SymbolLit *lit) { result = make_shared<core::LiteralType>(core::Symbols::Symbol(), lit->name); },
            [&](ast::Expression *expr) {
                ctx.state.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration, "Unsupported type literal");
                result = core::Types::dynamic();
            });
        ENFORCE(result.get() != nullptr);
        result->sanityCheck(ctx);
        return result;
    }

    shared_ptr<core::Type> interpretTCombinator(core::Context ctx, ast::Send *send) {
        switch (send->fun._id) {
            case core::Names::nilable()._id:
                return core::Types::buildOr(ctx, getResultType(ctx, send->args[0]), core::Types::nil());
            case core::Names::all()._id: {
                auto result = getResultType(ctx, send->args[0]);
                int i = 1;
                while (i < send->args.size()) {
                    result = core::Types::buildAnd(ctx, result, getResultType(ctx, send->args[i]));
                    i++;
                }
                return result;
            }
            case core::Names::any()._id: {
                auto result = getResultType(ctx, send->args[0]);
                int i = 1;
                while (i < send->args.size()) {
                    result = core::Types::buildOr(ctx, result, getResultType(ctx, send->args[i]));
                    i++;
                }
                return result;
            }
            case core::Names::enum_()._id: {
                if (send->args.size() != 1) {
                    ctx.state.error(send->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                    "enum only takes a single argument");
                    return core::Types::dynamic();
                }
                auto arr = ast::cast_tree<ast::Array>(send->args[0].get());
                if (arr == nullptr) {
                    // TODO(pay-server) unsilence this error and support enums from pay-server
                    { return core::Types::bottom(); }
                    ctx.state.error(send->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                    "enum must be passed a literal array. e.g. enum([1,\"foo\",MyClass])");
                    return core::Types::dynamic();
                }
                if (arr->elems.empty()) {
                    ctx.state.error(send->loc, core::errors::Resolver::InvalidTypeDeclaration, "enum([]) is invalid");
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
            case core::Names::untyped()._id:
                return core::Types::dynamic();

            case core::Names::arrayOf()._id: {
                auto elem = getResultType(ctx, send->args[0]);
                std::vector<shared_ptr<core::Type>> targs;
                targs.emplace_back(move(elem));
                return make_shared<core::AppliedType>(core::Symbols::Array(), move(targs));
            }
            case core::Names::hashOf()._id: {
                bool fail = (send->args.size() != 1);
                ast::Hash *hash;
                if (!fail) {
                    hash = ast::cast_tree<ast::Hash>(send->args[0].get());
                    fail = hash == nullptr;
                }
                int keyIndex = 0;
                int valueIndex = 1;
                // TODO: detect them(or not, as new syntax would not need this)
                if (!fail) {
                    auto key = getResultType(ctx, hash->values[keyIndex]);
                    auto value = getResultType(ctx, hash->values[valueIndex]);
                    std::vector<shared_ptr<core::Type>> targs;
                    targs.emplace_back(move(key));
                    targs.emplace_back(move(value));
                    return make_shared<core::AppliedType>(core::Symbols::Hash(), move(targs));
                }

                ctx.state.error(send->loc, core::errors::Resolver::InvalidTypeDeclaration, "Malformed hash type");
                return core::Types::dynamic();
            }

            case core::Names::noreturn()._id:
                return core::Types::bottom();

            default:
                ctx.state.error(send->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                "Unsupported type combinator {}", send->fun.toString(ctx));
                return core::Types::dynamic();
        }
    }

    shared_ptr<core::Type> getResultType(core::Context ctx, unique_ptr<ast::Expression> &expr) {
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
            [&](ast::Ident *i) {
                bool silenceGenericError = i->symbol == core::Symbols::Hash() || i->symbol == core::Symbols::Array() ||
                                           i->symbol == core::Symbols::Set() || i->symbol == core::Symbols::Struct() ||
                                           i->symbol == core::Symbols::File();
                // TODO: reduce this^^^ set.
                auto sym = dealiasSym(ctx, i->symbol);
                if (sym.data(ctx).isClass()) {
                    if (sym.data(ctx).typeMembers().empty()) {
                        result = make_shared<core::ClassType>(sym);
                    } else {
                        std::vector<shared_ptr<core::Type>> targs;
                        for (auto &UNUSED(arg) : sym.data(ctx).typeMembers()) {
                            targs.emplace_back(core::Types::dynamic());
                        }
                        result = make_shared<core::AppliedType>(sym, targs);
                        if (!silenceGenericError) {
                            ctx.state.error(i->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                            "Malformed type declaration. Generic class without type arguments {}",
                                            i->toString(ctx));
                        }
                    }
                } else if (sym.data(ctx).isTypeMember()) {
                    result = make_shared<core::LambdaParam>(sym);
                } else {
                    ctx.state.error(i->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                    "Malformed type declaration. Not a class type {}", i->toString(ctx));
                    result = core::Types::dynamic();
                }
            },
            [&](ast::Send *s) {
                auto *recvi = ast::cast_tree<ast::Ident>(s->recv.get());
                if (recvi == nullptr) {
                    ctx.state.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                    "Malformed type declaration. Unknown type syntax {}", expr->toString(ctx));
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

                if (s->fun != core::Names::squareBrackets()) {
                    ctx.state.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                    "Malformed type declaration. Unknown type syntax {}", expr->toString(ctx));
                }

                if (recvi->symbol == core::Symbols::T_Array()) {
                    if (s->args.size() != 1) {
                        ctx.state.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                        "Malformed T::Array[]: Expected 1 type argument");
                        result = core::Types::dynamic();
                        return;
                    }
                    auto elem = getResultType(ctx, s->args[0]);
                    std::vector<shared_ptr<core::Type>> targs;
                    targs.emplace_back(move(elem));
                    result = make_shared<core::AppliedType>(core::Symbols::Array(), move(targs));
                } else if (recvi->symbol == core::Symbols::T_Hash()) {
                    if (s->args.size() != 2) {
                        ctx.state.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                        "Malformed T::Hash[]: Expected 2 type arguments");
                        result = core::Types::dynamic();
                        return;
                    }
                    auto key = getResultType(ctx, s->args[0]);
                    auto value = getResultType(ctx, s->args[1]);
                    std::vector<shared_ptr<core::Type>> targs;
                    targs.emplace_back(move(key));
                    targs.emplace_back(move(value));
                    result = make_shared<core::AppliedType>(core::Symbols::Hash(), move(targs));
                } else if (recvi->symbol == core::Symbols::T_Proc()) {
                    if (s->args.empty()) {
                        ctx.state.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                        "Malformed T::Proc[]: Expected a return type");
                        result = core::Types::dynamic();
                        return;
                    }
                    auto arity = s->args.size() - 1;
                    if (arity > core::Symbols::MAX_PROC_ARITY) {
                        ctx.state.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                        "Malformed T::Proc[]: Too many arguments (max {})",
                                        core::Symbols::MAX_PROC_ARITY);
                        result = core::Types::dynamic();
                        return;
                    }
                    auto sym = core::Symbols::Proc(arity);
                    vector<shared_ptr<core::Type>> targs;

                    auto &back = s->args.back();
                    // extract return
                    shared_ptr<core::Type> returnType;
                    auto hash = ast::cast_tree<ast::Hash>(back.get());
                    if (hash != nullptr) {
                        const auto &r = find_if(hash->keys.begin(), hash->keys.end(), [](auto &expr) {
                            auto sym = ast::cast_tree<ast::SymbolLit>(expr.get());
                            return sym != nullptr && sym->name == core::Names::returns();
                        });
                        if (r != hash->keys.end()) {
                            auto idx = &*r - &hash->keys.front();
                            auto &val = hash->values[idx];
                            returnType = getResultType(ctx, val);
                        }
                    }
                    if (returnType == nullptr) {
                        ctx.state.error(back->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                        "Malformed type declaration. Expected `returns: <type>`");
                        result = core::Types::dynamic();
                        return;
                    }
                    targs.emplace_back(move(returnType));

                    for (auto &arg : s->args) {
                        if (&arg == &s->args.back()) {
                            continue;
                        }
                        targs.emplace_back(getResultType(ctx, arg));
                    }

                    result = make_shared<core::AppliedType>(sym, targs);
                } else {
                    auto &data = recvi->symbol.data(ctx);
                    if (s->args.size() != data.typeMembers().size()) {
                        ctx.state.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                        "Malformed {}[]: Expected %d type arguments, got %d", data.name.toString(ctx),
                                        data.typeMembers().size(), s->args.size());
                        result = core::Types::dynamic();
                        return;
                    }
                    std::vector<shared_ptr<core::Type>> targs;
                    for (auto &arg : s->args) {
                        auto elem = getResultType(ctx, arg);
                        targs.emplace_back(move(elem));
                    }
                    result = make_shared<core::AppliedType>(recvi->symbol, move(targs));
                }
            },
            [&](ast::Expression *expr) {
                ctx.state.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration, "Unsupported type syntax");
                result = core::Types::dynamic();
            });
        ENFORCE(result.get() != nullptr);
        result->sanityCheck(ctx);
        return result;
    }

    void fillInInfoFromSig(core::Context ctx, core::SymbolRef method, ast::Send *send, bool isOverloaded) {
        auto exprLoc = send->loc;
        auto &methodInfo = method.data(ctx);

        struct {
            bool sig = false;
            bool args = false;
            bool abstract = false;
            bool override_ = false;
            bool overridable = false;
            bool implementation = false;
            bool returns = false;
            bool checked = false;
        } seen;

        while (send != nullptr) {
            switch (send->fun._id) {
                case core::Names::sig()._id: {
                    seen.sig = true;
                    if (send->args.empty()) {
                        break;
                    }
                    seen.args = true;

                    if (send->args.size() > 1) {
                        ctx.state.error(send->loc, core::errors::Resolver::InvalidMethodSignature,
                                        "Wrong number of args to `sig`. Got {}, expected 0-1", send->args.size());
                    }
                    auto *hash = ast::cast_tree<ast::Hash>(send->args[0].get());
                    if (hash == nullptr) {
                        ctx.state.error(send->loc, core::errors::Resolver::InvalidMethodSignature,
                                        "Malformed `sig`; Expected a hash of arguments => types.", send->args.size());
                        break;
                    }

                    int i = 0;
                    for (auto &key : hash->keys) {
                        auto &value = hash->values[i++];
                        if (auto *symbolLit = ast::cast_tree<ast::SymbolLit>(key.get())) {
                            auto fnd = find_if(
                                methodInfo.arguments().begin(), methodInfo.arguments().end(),
                                [&](core::SymbolRef sym) -> bool { return sym.data(ctx).name == symbolLit->name; });
                            if (fnd == methodInfo.arguments().end()) {
                                ctx.state.error(key->loc, core::errors::Resolver::InvalidMethodSignature,
                                                "Malformed `sig`. Unknown argument name {}", key->toString(ctx));
                            } else {
                                core::SymbolRef arg = *fnd;
                                arg.data(ctx).resultType = getResultType(ctx, value);
                                arg.data(ctx).definitionLoc = key->loc;
                            }
                        }
                    }
                    break;
                }
                case core::Names::abstract()._id:
                    seen.abstract = true;
                    break;
                case core::Names::override_()._id:
                    seen.override_ = true;
                    break;
                case core::Names::implementation()._id:
                    seen.implementation = true;
                    break;
                case core::Names::overridable()._id:
                    seen.overridable = true;
                    break;
                case core::Names::returns()._id:
                    seen.returns = true;
                    if (send->args.size() != 1) {
                        ctx.state.error(send->loc, core::errors::Resolver::InvalidMethodSignature,
                                        "Wrong number of args to `sig.returns`. Got {}, expected 1", send->args.size());
                    }
                    if (!send->args.empty()) {
                        methodInfo.resultType = getResultType(ctx, send->args.front());
                    }

                    break;
                case core::Names::checked()._id:
                    seen.checked = true;
                    break;
                default:
                    ctx.state.error(send->loc, core::errors::Resolver::InvalidMethodSignature,
                                    "Unknown `sig` builder method {}.", send->fun.toString(ctx));
            }
            send = ast::cast_tree<ast::Send>(send->recv.get());
        }
        ENFORCE(seen.sig);

        if (!seen.returns) {
            if (seen.args ||
                !(seen.abstract || seen.override_ || seen.implementation || seen.overridable || seen.abstract)) {
                ctx.state.error(exprLoc, core::errors::Resolver::InvalidMethodSignature,
                                "Malformed `sig`: No return type specified. Specify one with .returns()");
            }
        }

        for (auto it = methodInfo.arguments().begin(); it != methodInfo.arguments().end(); /* nothing */) {
            core::SymbolRef arg = *it;
            if (isOverloaded && arg.data(ctx).isKeyword()) {
                ctx.state.error(arg.data(ctx).definitionLoc, core::errors::Resolver::InvalidMethodSignature,
                                "Malformed sig. Overloaded functions cannot have keyword arguments:  {}",
                                arg.data(ctx).name.toString(ctx));
            }
            if (arg.data(ctx).resultType.get() != nullptr) {
                ++it;
            } else {
                if (isOverloaded) {
                    it = methodInfo.arguments().erase(it);
                } else {
                    arg.data(ctx).resultType = core::Types::dynamic();
                    if (seen.args || seen.returns) {
                        // Only error if we have any types
                        ctx.state.error(arg.data(ctx).definitionLoc, core::errors::Resolver::InvalidMethodSignature,
                                        "Malformed sig. Type not specified for argument {}",
                                        arg.data(ctx).name.toString(ctx));
                    }
                }
            }
        }
    }

    bool isSig(core::Context ctx, ast::Send *send) {
        while (send != nullptr) {
            if (send->fun == core::Names::sig() && ast::cast_tree<ast::Self>(send->recv.get()) != nullptr) {
                return true;
            }
            send = ast::cast_tree<ast::Send>(send->recv.get());
        }
        return false;
    }

    void processClassBody(core::Context ctx, ast::ClassDef *klass) {
        InlinedVector<unique_ptr<ast::Expression>, 1> lastSig;
        for (auto &stat : klass->rhs) {
            typecase(stat.get(),

                     [&](ast::Send *send) {
                         if (isSig(ctx, send)) {
                             if (!lastSig.empty()) {
                                 if (!ctx.withOwner(klass->symbol).permitOverloadDefinitions()) {
                                     ctx.state.error(core::ComplexError(
                                         lastSig[0]->loc, core::errors::Resolver::InvalidMethodSignature,
                                         "Unused type annotation. No method def before next annotation.",
                                         core::ErrorLine(send->loc, "Type annotation that will be used instead.")));
                                 }
                             }
                             lastSig.emplace_back(move(stat));
                             return;
                         }

                         if (!ast::isa_tree<ast::Self>(send->recv.get())) {
                             return;
                         }

                         switch (send->fun._id) {
                             case core::Names::declareVariables()._id:
                                 processDeclareVariables(ctx.withOwner(klass->symbol), send);
                                 break;
                             case core::Names::attr()._id:
                             case core::Names::attrReader()._id:
                                 defineAttr(ctx.withOwner(klass->symbol), send, true, false);
                                 break;
                             case core::Names::attrWriter()._id:
                                 defineAttr(ctx.withOwner(klass->symbol), send, false, true);
                                 break;

                             case core::Names::attrAccessor()._id:
                                 defineAttr(ctx.withOwner(klass->symbol), send, true, true);
                                 break;
                             default:
                                 return;
                         }
                         stat.reset(nullptr);
                     },

                     [&](ast::MethodDef *mdef) {
                         if (!lastSig.empty()) {
                             counterInc("types.sig.count");

                             bool isOverloaded =
                                 lastSig.size() > 1 && ctx.withOwner(klass->symbol).permitOverloadDefinitions();

                             if (isOverloaded) {
                                 mdef->symbol.data(ctx).setOverloaded();
                                 int i = 1;

                                 while (i < lastSig.size()) {
                                     auto overload = ctx.state.enterNewMethodOverload(lastSig[i]->loc, mdef->symbol, i);
                                     fillInInfoFromSig(ctx, overload, ast::cast_tree<ast::Send>(lastSig[i].get()),
                                                       isOverloaded);
                                     if (i + 1 < lastSig.size()) {
                                         overload.data(ctx).setOverloaded();
                                     }
                                     i++;
                                 }
                             }

                             fillInInfoFromSig(ctx, mdef->symbol, ast::cast_tree<ast::Send>(lastSig[0].get()),
                                               isOverloaded);

                             // OVERLOAD
                             lastSig.clear();
                         }

                     },
                     [&](ast::ClassDef *cdef) {
                         // Leave in place
                     },

                     [&](ast::Assign *assgn) {
                         if (ast::Ident *id = ast::cast_tree<ast::Ident>(assgn->lhs.get())) {
                             if (id->symbol.data(ctx).name.data(ctx).kind == core::CONSTANT) {
                                 stat.reset(nullptr);
                             }
                         }
                     },

                     [&](ast::EmptyTree *e) { stat.reset(nullptr); },

                     [&](ast::Expression *e) {});
        }

        if (!lastSig.empty()) {
            ctx.state.error(lastSig[0]->loc, core::errors::Resolver::InvalidMethodSignature,
                            "Malformed sig. No method def following it.");
        }

        auto toRemove = remove_if(klass->rhs.begin(), klass->rhs.end(),
                                  [](unique_ptr<ast::Expression> &stat) -> bool { return stat.get() == nullptr; });

        klass->rhs.erase(toRemove, klass->rhs.end());
    }

    core::SymbolRef dealiasSym(core::Context ctx, core::SymbolRef sym) {
        while (sym.data(ctx).isStaticField()) {
            auto *ct = core::cast_type<core::ClassType>(sym.data(ctx).resultType.get());
            if (ct == nullptr) {
                break;
            }
            auto klass = ct->symbol.data(ctx).attachedClass(ctx);
            if (!klass.exists()) {
                break;
            }

            sym = klass;
        }
        return sym;
    }

    // Resolve the type of the rhs of a constant declaration. This logic is
    // extremely simplistic; We only handle simple literals, and explicit casts.
    //
    // We don't handle array or hash literals, because intuiting the element
    // type (once we have generics) will be nontrivial.
    shared_ptr<core::Type> resolveConstantType(core::Context ctx, unique_ptr<ast::Expression> &expr) {
        shared_ptr<core::Type> result;
        typecase(expr.get(), [&](ast::SymbolLit *) { result = core::Types::Symbol(); },
                 [&](ast::FloatLit *) { result = core::Types::Float(); },
                 [&](ast::IntLit *) { result = core::Types::Integer(); },
                 [&](ast::StringLit *) { result = core::Types::String(); },
                 [&](ast::BoolLit *b) {
                     if (b->value) {
                         result = core::Types::trueClass();
                     } else {
                         result = core::Types::falseClass();
                     }
                 },
                 [&](ast::Cast *cast) {
                     if (cast->assertType) {
                         ctx.state.error(cast->loc, core::errors::Resolver::ConstantAssertType,
                                         "Use cast() instead of assert_type!() to specify the type of constants.");
                     }
                     result = cast->type;
                 },
                 [&](ast::Expression *) { result = core::Types::dynamic(); });
        return result;
    }

public:
    ast::Assign *postTransformAssign(core::Context ctx, ast::Assign *asgn) {
        auto *id = ast::cast_tree<ast::Ident>(asgn->lhs.get());
        if (id == nullptr) {
            return asgn;
        }

        auto &data = id->symbol.data(ctx);
        if (data.isTypeMember()) {
            ENFORCE(data.isFixed());
            auto send = ast::cast_tree<ast::Send>(asgn->rhs.get());
            auto recv = ast::cast_tree<ast::Ident>(send->recv.get());
            ENFORCE(recv->symbol == core::Symbols::T());
            ENFORCE(send->fun == core::Names::typeDecl());
            int arg;
            if (send->args.size() == 1) {
                arg = 0;
            } else if (send->args.size() == 2) {
                arg = 1;
            } else {
                Error::raise("Wrong arg count");
            }

            auto *hash = ast::cast_tree<ast::Hash>(send->args[arg].get());
            if (hash) {
                int i = -1;
                for (auto &keyExpr : hash->keys) {
                    i++;
                    auto *key = ast::cast_tree<ast::SymbolLit>(keyExpr.get());
                    if (key && key->name == core::Names::fixed()) {
                        data.resultType = getResultType(ctx, hash->values[i]);
                    }
                }
            }
        } else if (data.isStaticField()) {
            if (data.resultType != nullptr) {
                return asgn;
            }
            data.resultType = resolveConstantType(ctx, asgn->rhs);
        }

        return asgn;
    }

    ast::Expression *postTransformClassDef(core::Context ctx, ast::ClassDef *original) {
        processClassBody(ctx, original);
        return original;
    }

    ast::Expression *postTransformSend(core::Context ctx, ast::Send *send) {
        auto *id = ast::cast_tree<ast::Ident>(send->recv.get());
        if (id == nullptr) {
            return send;
        }
        if (id->symbol != core::Symbols::T()) {
            return send;
        }
        bool checked = false;
        switch (send->fun._id) {
            case core::Names::assertType()._id:
                checked = true;
                /* fallthrough */
            case core::Names::cast()._id: {
                if (send->args.size() < 2) {
                    ctx.state.error(send->loc, core::errors::Resolver::InvalidCast,
                                    "Not enough arguments to {}: got {}, expected 2", send->fun.toString(ctx),
                                    send->args.size());
                    return send;
                }

                auto expr = move(send->args[0]);
                auto type = getResultType(ctx, send->args[1]);
                return new ast::Cast(send->loc, type, move(expr), checked);
            }
            default:
                return send;
        }
    }
}; // namespace namer

class FlattenWalk {
public:
    FlattenWalk() {
        newMethodSet();
    }
    ~FlattenWalk() {
        ENFORCE(methodScopes.empty());
        ENFORCE(classes.empty());
        ENFORCE(classStack.empty());
    }

    ast::ClassDef *preTransformClassDef(core::Context ctx, ast::ClassDef *classDef) {
        newMethodSet();
        classStack.emplace_back(classes.size());
        classes.emplace_back();
        return classDef;
    }

    ast::Expression *postTransformClassDef(core::Context ctx, ast::ClassDef *classDef) {
        ENFORCE(!classStack.empty());
        ENFORCE(classes.size() > classStack.back());
        ENFORCE(classes[classStack.back()] == nullptr);

        auto rhs = addMethods(ctx, move(classDef->rhs));
        classes[classStack.back()] = make_unique<ast::ClassDef>(classDef->loc, classDef->symbol, move(classDef->name),
                                                                move(classDef->ancestors), move(rhs), classDef->kind);
        classStack.pop_back();
        return new ast::EmptyTree(classDef->loc);
    };

    ast::MethodDef *preTransformMethodDef(core::Context ctx, ast::MethodDef *methodDef) {
        auto &methods = curMethodSet();
        methods.stack.emplace_back(methods.methods.size());
        methods.methods.emplace_back();
        return methodDef;
    }

    ast::Expression *postTransformMethodDef(core::Context ctx, ast::MethodDef *methodDef) {
        auto &methods = curMethodSet();
        ENFORCE(!methods.stack.empty());
        ENFORCE(methods.methods.size() > methods.stack.back());
        ENFORCE(methods.methods[methods.stack.back()] == nullptr);

        methods.methods[methods.stack.back()] =
            make_unique<ast::MethodDef>(methodDef->loc, methodDef->symbol, methodDef->name, move(methodDef->args),
                                        move(methodDef->rhs), methodDef->isSelf);
        methods.stack.pop_back();
        return new ast::EmptyTree(methodDef->loc);
    };

    std::unique_ptr<ast::Expression> addClasses(core::Context ctx, std::unique_ptr<ast::Expression> tree) {
        if (classes.empty()) {
            ENFORCE(sortedClasses().empty());
            return tree;
        }
        if (classes.size() == 1 && (ast::cast_tree<ast::EmptyTree>(tree.get()) != nullptr)) {
            // It was only 1 class to begin with, put it back
            return move(sortedClasses()[0]);
        }

        auto insSeq = ast::cast_tree<ast::InsSeq>(tree.get());
        if (insSeq == nullptr) {
            ast::InsSeq::STATS_store stats;
            tree = make_unique<ast::InsSeq>(tree->loc, move(stats), move(tree));
            return addClasses(ctx, move(tree));
        }

        for (auto &clas : sortedClasses()) {
            ENFORCE(!!clas);
            insSeq->stats.emplace_back(move(clas));
        }
        return tree;
    }

    std::unique_ptr<ast::Expression> addMethods(core::Context ctx, std::unique_ptr<ast::Expression> tree) {
        auto &methods = curMethodSet().methods;
        if (methods.empty()) {
            ENFORCE(popCurMethodDefs().empty());
            return tree;
        }
        if (methods.size() == 1 && (ast::cast_tree<ast::EmptyTree>(tree.get()) != nullptr)) {
            // It was only 1 method to begin with, put it back
            unique_ptr<ast::Expression> methodDef = move(popCurMethodDefs()[0]);
            return methodDef;
        }

        auto insSeq = ast::cast_tree<ast::InsSeq>(tree.get());
        if (insSeq == nullptr) {
            ast::InsSeq::STATS_store stats;
            tree = make_unique<ast::InsSeq>(tree->loc, move(stats), move(tree));
            return addMethods(ctx, move(tree));
        }

        for (auto &method : popCurMethodDefs()) {
            ENFORCE(!!method);
            insSeq->stats.emplace_back(move(method));
        }
        return tree;
    }

private:
    vector<unique_ptr<ast::ClassDef>> sortedClasses() {
        ENFORCE(classStack.empty());
        auto ret = move(classes);
        classes.clear();
        return ret;
    }

    ast::ClassDef::RHS_store addMethods(core::Context ctx, ast::ClassDef::RHS_store rhs) {
        if (curMethodSet().methods.size() == 1 && rhs.size() == 1 &&
            (ast::cast_tree<ast::EmptyTree>(rhs[0].get()) != nullptr)) {
            // It was only 1 method to begin with, put it back
            rhs.pop_back();
            rhs.emplace_back(move(popCurMethodDefs()[0]));
            return rhs;
        }
        for (auto &method : popCurMethodDefs()) {
            ENFORCE(!!method);
            rhs.emplace_back(move(method));
        }
        return rhs;
    }

    vector<unique_ptr<ast::MethodDef>> popCurMethodDefs() {
        auto ret = move(curMethodSet().methods);
        ENFORCE(curMethodSet().stack.empty());
        popCurMethodSet();
        return ret;
    };

    struct Methods {
        vector<unique_ptr<ast::MethodDef>> methods;
        vector<int> stack;
        Methods() = default;
    };
    void newMethodSet() {
        methodScopes.emplace_back();
    }
    Methods &curMethodSet() {
        ENFORCE(!methodScopes.empty());
        return methodScopes.back();
    }
    void popCurMethodSet() {
        ENFORCE(!methodScopes.empty());
        methodScopes.pop_back();
    }

    // We flatten nested classes and methods into a flat list. We want to sort
    // them by their starts, so that `class A; class B; end; end` --> `class A;
    // end; class B; end`.
    //
    // In order to make TreeMap work out, we can't remove them from the AST
    // until the `postTransform*` hook. Appending them to a list at that point
    // would result in an "bottom-up" ordering, so instead we store a stack of
    // "where does the next definition belong" into `classStack` and
    // `methodScopes.stack`, which we push onto in the `preTransform* hook, and
    // pop from in the `postTransform` hook.

    vector<Methods> methodScopes;
    vector<unique_ptr<ast::ClassDef>> classes;
    vector<int> classStack;
};

class ResolveVariablesWalk {
public:
    ast::Expression *postTransformUnresolvedIdent(core::Context ctx, ast::UnresolvedIdent *id) {
        core::SymbolRef klass;

        switch (id->kind) {
            case ast::UnresolvedIdent::Class:
                klass = ctx.contextClass();
                break;
            case ast::UnresolvedIdent::Instance:
                klass = ctx.selfClass();
                break;
            default:
                // These should have been removed in the namer
                Error::notImplemented();
        }

        core::SymbolRef sym = klass.data(ctx).findMemberTransitive(ctx, id->name);
        if (!sym.exists()) {
            ctx.state.error(id->loc, core::errors::Resolver::UndeclaredVariable, "Use of undeclared variable `{}'",
                            id->name.toString(ctx));
            if (id->kind == ast::UnresolvedIdent::Class) {
                sym = ctx.state.enterStaticFieldSymbol(id->loc, klass, id->name);
            } else {
                sym = ctx.state.enterFieldSymbol(id->loc, klass, id->name);
            }
            sym.data(ctx).resultType = core::Types::dynamic();
        }

        return new ast::Ident(id->loc, sym);
    };
};

class ResolveSanityCheckWalk {
public:
    ast::Expression *postTransformClassDef(core::Context ctx, ast::ClassDef *original) {
        ENFORCE(original->symbol != core::Symbols::todo());
        return original;
    }
    ast::Expression *postTransformMethodDef(core::Context ctx, ast::MethodDef *original) {
        ENFORCE(original->symbol != core::Symbols::todo());
        return original;
    }
    ast::Expression *postTransformConstDef(core::Context ctx, ast::ConstDef *original) {
        ENFORCE(original->symbol != core::Symbols::todo());
        return original;
    }
    ast::Expression *postTransformIdent(core::Context ctx, ast::Ident *original) {
        ENFORCE(original->symbol != core::Symbols::todo());
        return original;
    }
    ast::Expression *postTransformUnresolvedIdent(core::Context ctx, ast::UnresolvedIdent *original) {
        Error::raise("These should have all been removed");
    }
    ast::Expression *postTransformSelf(core::Context ctx, ast::Self *original) {
        ENFORCE(original->claz != core::Symbols::todo());
        return original;
    }
    ast::Expression *postTransformBlock(core::Context ctx, ast::Block *original) {
        ENFORCE(original->symbol != core::Symbols::todo());
        return original;
    }
};

std::vector<std::unique_ptr<ast::Expression>> Resolver::run(core::Context ctx,
                                                            std::vector<std::unique_ptr<ast::Expression>> trees) {
    ResolveConstantsWalk constants(ctx);
    for (auto &tree : trees) {
        tree = ast::TreeMap<ResolveConstantsWalk>::apply(ctx, constants, move(tree));
    }
    ResolveSignaturesWalk sigs;
    ResolveVariablesWalk vars;

    for (auto &tree : trees) {
        tree = ast::TreeMap<ResolveSignaturesWalk>::apply(ctx, sigs, move(tree));
        tree = ast::TreeMap<ResolveVariablesWalk>::apply(ctx, vars, move(tree));

        // declared in here since it holds onto state
        FlattenWalk flatten;
        tree = ast::TreeMap<FlattenWalk>::apply(ctx, flatten, move(tree));
        tree = flatten.addClasses(ctx, move(tree));
        tree = flatten.addMethods(ctx, move(tree));
    }

    finalizeResolution(ctx.state);

    if (debug_mode) {
        ResolveSanityCheckWalk sanity;
        for (auto &tree : trees) {
            tree = ast::TreeMap<ResolveSanityCheckWalk>::apply(ctx, sanity, move(tree));
        }
    }

    return trees;
} // namespace namer

} // namespace resolver
} // namespace ruby_typer
