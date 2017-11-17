#include "../ast/ast.h"
#include "ast/ast.h"
#include "core/core.h"
#include "namer/namer.h"

#include <algorithm> // find_if
#include <list>
#include <vector>

using namespace std;

namespace ruby_typer {
namespace namer {

struct Nesting {
    unique_ptr<Nesting> parent;
    core::SymbolRef scope;

    Nesting(unique_ptr<Nesting> parent, core::SymbolRef scope) : parent(move(parent)), scope(scope) {}

    Nesting(const Nesting &rhs) = delete;
    Nesting(Nesting &&rhs) = delete;
};

class ResolveWalk {
private:
    core::SymbolRef resolveLhs(core::Context ctx, core::NameRef name) {
        Nesting *scope = nesting_.get();
        while (scope != nullptr) {
            auto lookup = scope->scope.info(ctx).findMember(name);
            if (lookup.exists()) {
                return lookup;
            }
            scope = scope->parent.get();
        }
        return core::SymbolRef(0);
    }

    core::SymbolRef resolveConstant(core::Context ctx, ast::ConstantLit *c) {
        if (ast::cast_tree<ast::EmptyTree>(c->scope.get()) != nullptr) {
            core::SymbolRef result = resolveLhs(ctx, c->cnst);
            if (result.exists()) {
                return result;
            }
            ctx.state.errors.error(c->loc, core::ErrorClass::StubConstant, "Stubbing out unknown constant {}",
                                   c->toString(ctx));
            return core::GlobalState::defn_untyped();

        } else if (ast::ConstantLit *scope = ast::cast_tree<ast::ConstantLit>(c->scope.get())) {
            auto resolved = resolveConstant(ctx, scope);
            if (!resolved.exists() || resolved == core::GlobalState::defn_untyped())
                return resolved;
            core::SymbolRef result = resolved.info(ctx).findMember(c->cnst);
            if (!result.exists()) {
                ctx.state.errors.error(c->loc, core::ErrorClass::StubConstant, "Stubbing out unknown constant {}",
                                       c->toString(ctx));
                result = core::GlobalState::defn_untyped();
            }
            c->scope = make_unique<ast::Ident>(c->loc, resolved);

            return result;
        } else {
            ctx.state.errors.error(c->loc, core::ErrorClass::DynamicConstant,
                                   "Dynamic constant references are unsupported {}", c->toString(ctx));
            return core::GlobalState::defn_untyped();
        }
    }

    unique_ptr<ast::Expression> maybeResolve(core::Context ctx, ast::Expression *expr) {
        if (ast::ConstantLit *cnst = ast::cast_tree<ast::ConstantLit>(expr)) {
            core::SymbolRef resolved = resolveConstant(ctx, cnst);
            if (resolved.exists())
                return make_unique<ast::Ident>(expr->loc, resolved);
        }
        return nullptr;
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
                result = make_shared<core::ArrayType>(elems);
            },
            [&](ast::Ident *i) {
                if (i->symbol.info(ctx).isClass()) {
                    result = make_shared<core::ClassType>(i->symbol);
                } else {
                    ctx.state.errors.error(i->loc, core::ErrorClass::InvalidTypeDeclaration,
                                           "Malformed type declaration. Not a class type {}", i->toString(ctx));
                    result = core::Types::dynamic();
                }
            },
            [&](ast::Send *s) {
                if (auto *recvi = ast::cast_tree<ast::Ident>(s->recv.get())) {
                    if (recvi->symbol != core::GlobalState::defn_Opus_Types()) {
                        ctx.state.errors.error(recvi->loc, core::ErrorClass::InvalidTypeDeclaration,
                                               "Misformed type declaration. Unknown argument type type {}",
                                               expr->toString(ctx));
                        result = core::Types::dynamic();
                    } else {
                        if (s->fun == core::Names::nilable()) {
                            result = make_shared<core::OrType>(getResultType(ctx, s->args[0]), core::Types::nil());
                        } else if (s->fun == core::Names::all()) {
                            result = getResultType(ctx, s->args[0]);
                            int i = 1;
                            while (i < s->args.size()) {
                                result = make_shared<core::AndType>(result, getResultType(ctx, s->args[i]));
                                i++;
                            }
                        } else if (s->fun == core::Names::any()) {
                            result = getResultType(ctx, s->args[0]);
                            int i = 1;
                            while (i < s->args.size()) {
                                result = make_shared<core::OrType>(result, getResultType(ctx, s->args[i]));
                                i++;
                            }
                        } else {
                            ctx.state.errors.error(s->loc, core::ErrorClass::InvalidTypeDeclaration,
                                                   "Unsupported type combinator {}", s->fun.toString(ctx));
                            result = core::Types::dynamic();
                        }
                    }
                } else {
                    ctx.state.errors.error(expr->loc, core::ErrorClass::InvalidTypeDeclaration,
                                           "Misformed type declaration. Unknown type syntax {}", expr->toString(ctx));
                    result = core::Types::dynamic();
                }
            },
            [&](ast::Expression *expr) {
                ctx.state.errors.error(expr->loc, core::ErrorClass::InvalidTypeDeclaration, "Unsupported type syntax");
                result = core::Types::dynamic();
            });
        Error::check(result.get() != nullptr);
        return result;
    }

    void fillInInfoFromStandardMethod(core::Context ctx, core::Symbol &methoInfo,
                                      unique_ptr<ast::Send> &lastStandardMethod, int argsSize) {
        if (ast::cast_tree<ast::Self>(lastStandardMethod->recv.get()) == nullptr ||
            lastStandardMethod->block != nullptr) {
            ctx.state.errors.error(lastStandardMethod->loc, core::ErrorClass::InvalidMethodSignature,
                                   "Misformed standard_method " + lastStandardMethod->toString(ctx));
        } else {
            for (auto &arg : lastStandardMethod->args) {
                if (auto *hash = ast::cast_tree<ast::Hash>(arg.get())) {
                    int i = 0;
                    for (unique_ptr<ast::Expression> &key : hash->keys) {
                        unique_ptr<ast::Expression> &value = hash->values[i++];
                        if (auto *symbolLit = ast::cast_tree<ast::SymbolLit>(key.get())) {
                            if (symbolLit->name == core::Names::returns()) {
                                // fill in return type
                                methoInfo.resultType = getResultType(ctx, value);
                                methoInfo.definitionLoc = value->loc;
                            } else {
                                auto fnd = find_if(
                                    methoInfo.arguments().begin(), methoInfo.arguments().end(),
                                    [&](core::SymbolRef sym) -> bool { return sym.info(ctx).name == symbolLit->name; });
                                if (fnd == methoInfo.arguments().end()) {
                                    ctx.state.errors.error(key->loc, core::ErrorClass::InvalidMethodSignature,
                                                           "Misformed standard_method. Unknown argument name type {}",
                                                           key->toString(ctx));
                                } else {
                                    core::SymbolRef arg = *fnd;
                                    arg.info(ctx).resultType = getResultType(ctx, value);
                                    arg.info(ctx).definitionLoc = key->loc;
                                }
                            }
                        } else {
                            ctx.state.errors.error(key->loc, core::ErrorClass::InvalidMethodSignature,
                                                   "Misformed standard_method. Unknown key type {}",
                                                   key->toString(ctx));
                        }
                    }
                }
            }
        }
    }

    void processDeclareVariables(core::Context ctx, ast::Send *send) {
        if (ast::cast_tree<ast::Self>(send->recv.get()) == nullptr || send->block != nullptr) {
            ctx.state.errors.error(send->loc, core::ErrorClass::InvalidDeclareVariables,
                                   "Malformed `declare_variables'");
            return;
        }

        if (send->args.size() != 1) {
            ctx.state.errors.error(send->loc, core::ErrorClass::InvalidDeclareVariables,
                                   "Wrong number of arguments to `declare_variables'");
            return;
        }
        auto hash = ast::cast_tree<ast::Hash>(send->args.front().get());
        if (hash == nullptr) {
            ctx.state.errors.error(send->loc, core::ErrorClass::InvalidDeclareVariables,
                                   "Malformed `declare_variables': Argument must be a hash");
            return;
        }
        for (int i = 0; i < hash->keys.size(); ++i) {
            auto &key = hash->keys[i];
            auto &value = hash->values[i];
            auto sym = ast::cast_tree<ast::SymbolLit>(key.get());
            if (sym == nullptr) {
                ctx.state.errors.error(key->loc, core::ErrorClass::InvalidDeclareVariables,
                                       "`declare_variables': variable names must be symbols");
                continue;
            }

            auto typ = getResultType(ctx, value);
            core::SymbolRef var;

            auto str = sym->name.toString(ctx);
            if (str.substr(0, 2) == "@@") {
                core::Symbol &info = ctx.owner.info(ctx);
                var = info.findMember(sym->name);
                if (var.exists()) {
                    ctx.state.errors.error(key->loc, core::ErrorClass::DuplicateVariableDeclaration,
                                           "Redeclaring variable `{}'", str);
                } else {
                    var = ctx.state.enterStaticFieldSymbol(sym->loc, ctx.owner, sym->name);
                }
            } else if (str.substr(0, 1) == "@") {
                core::Symbol &info = ctx.owner.info(ctx);
                var = info.findMember(sym->name);
                if (var.exists()) {
                    ctx.state.errors.error(key->loc, core::ErrorClass::DuplicateVariableDeclaration,
                                           "Redeclaring variable `{}'", str);
                } else {
                    var = ctx.state.enterFieldSymbol(sym->loc, ctx.owner, sym->name);
                }
            } else {
                ctx.state.errors.error(key->loc, core::ErrorClass::InvalidDeclareVariables,
                                       "`declare_variables`: variables must start with @ or @@");
            }

            if (var.exists())
                var.info(ctx).resultType = typ;
        }
    }

public:
    ResolveWalk(core::Context ctx) : nesting_(make_unique<Nesting>(nullptr, ctx.state.defn_root())) {}

    ast::ClassDef *preTransformClassDef(core::Context ctx, ast::ClassDef *original) {
        nesting_ = make_unique<Nesting>(move(nesting_), original->symbol);
        return original;
    }
    ast::Expression *postTransformClassDef(core::Context ctx, ast::ClassDef *original) {
        nesting_ = move(nesting_->parent);

        for (auto &ancst : original->ancestors) {
            if (auto resolved = maybeResolve(ctx, ancst.get()))
                ancst.swap(resolved);
        }
        core::Symbol &info = original->symbol.info(ctx);
        if (original->ancestors.size() > 0) {
            core::Symbol &info = original->symbol.info(ctx);
            for (auto &ancst : original->ancestors) {
                ast::Ident *id = ast::cast_tree<ast::Ident>(ancst.get());
                if (id == nullptr || !id->symbol.info(ctx).isClass()) {
                    ctx.state.errors.error(id->loc, core::ErrorClass::DynamicSuperclass,
                                           "Superclasses and mixins must be statically resolved.");
                    continue;
                }
                info.argumentsOrMixins.emplace_back(id->symbol);
                if (&ancst == &original->ancestors.front()) {
                    if (!info.superClass.exists() || info.superClass == core::GlobalState::defn_object() ||
                        info.superClass == id->symbol) {
                        info.superClass = id->symbol;
                    } else {
                        ctx.state.errors.error(id->loc, core::ErrorClass::RedefinitionOfParents,
                                               "Class parents redefined for class {}",
                                               original->symbol.info(ctx).name.toString(ctx));
                    }
                }
            }
        } else if (!info.superClass.exists() && original->symbol != core::GlobalState::defn_Basic_Object() &&
                   original->symbol != core::GlobalState::defn_Kernel() &&
                   original->symbol != core::GlobalState::defn_object() &&
                   !core::GlobalState::defn_object().info(ctx).derivesFrom(ctx, original->symbol) &&
                   original->kind != ast::ClassDefKind::Module && original->kind) {
            info.superClass = core::GlobalState::defn_object();
            info.argumentsOrMixins.emplace_back(core::GlobalState::defn_object());
        }

        unique_ptr<ast::Send> lastStandardMethod;
        for (auto &stat : original->rhs) {
            if (auto send = ast::cast_tree<ast::Send>(stat.get())) {
                if (send->fun == core::Names::standardMethod()) {
                    lastStandardMethod.reset(send);
                    stat.release();
                } else if (send->fun == core::Names::declareVariables()) {
                    processDeclareVariables(ctx.withOwner(original->symbol), send);
                }
            } else if (auto mdef = ast::cast_tree<ast::MethodDef>(stat.get())) {
                if (lastStandardMethod) {
                    core::Symbol &methoInfo = mdef->symbol.info(ctx);
                    fillInInfoFromStandardMethod(ctx, methoInfo, lastStandardMethod, mdef->args.size());
                    lastStandardMethod.reset(nullptr);
                }
            }
        }

        auto toRemove = remove_if(original->rhs.begin(), original->rhs.end(),
                                  [](unique_ptr<ast::Expression> &stat) -> bool { return stat.get() == nullptr; });

        original->rhs.erase(toRemove, original->rhs.end());

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

    unique_ptr<Nesting> nesting_;
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

        core::SymbolRef sym = klass.info(ctx).findMemberTransitive(ctx, id->name);
        if (!sym.exists()) {
            ctx.state.errors.error(id->loc, core::ErrorClass::UndeclaredVariable, "Use of undeclared variable `{}'",
                                   id->name.toString(ctx));
            if (id->kind == ast::UnresolvedIdent::Class) {
                sym = ctx.state.enterStaticFieldSymbol(id->loc, klass, id->name);
            } else {
                sym = ctx.state.enterFieldSymbol(id->loc, klass, id->name);
            }
            sym.info(ctx).resultType = core::Types::dynamic();
        }

        return new ast::Ident(id->loc, sym);
    };
};

unique_ptr<ast::Expression> Resolver::run(core::Context &ctx, unique_ptr<ast::Expression> tree) {
    ResolveWalk walk(ctx);
    tree = ast::TreeMap<ResolveWalk>::apply(ctx, walk, move(tree));
    ResolveVariablesWalk vars;
    tree = ast::TreeMap<ResolveVariablesWalk>::apply(ctx, vars, move(tree));
    return tree;
}
} // namespace namer
} // namespace ruby_typer
