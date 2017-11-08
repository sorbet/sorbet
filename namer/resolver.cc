#include "../ast/ast.h"
#include "ast/ast.h"
#include "namer/namer.h"

#include <algorithm> // find_if
#include <list>
#include <vector>

using namespace std;

namespace ruby_typer {
namespace namer {

struct Nesting {
    unique_ptr<Nesting> parent;
    ast::SymbolRef scope;

    Nesting(unique_ptr<Nesting> parent, ast::SymbolRef scope) : parent(move(parent)), scope(scope) {}

    Nesting(const Nesting &rhs) = delete;
    Nesting(Nesting &&rhs) = delete;
};

class ResolveWalk {
private:
    ast::SymbolRef resolveLhs(ast::Context ctx, ast::NameRef name) {
        Nesting *scope = nesting_.get();
        while (scope != nullptr) {
            auto lookup = scope->scope.info(ctx).findMember(name);
            if (lookup.exists()) {
                return lookup;
            }
            scope = scope->parent.get();
        }
        return ast::SymbolRef(0);
    }

    ast::SymbolRef resolveConstant(ast::Context ctx, ast::ConstantLit *c) {
        if (dynamic_cast<ast::EmptyTree *>(c->scope.get()) != nullptr) {
            ast::SymbolRef result = resolveLhs(ctx, c->cnst);
            if (result.exists()) {
                return result;
            }
            ctx.state.errors.error(c->loc, ast::ErrorClass::StubConstant,
                                   "Stubbing out unknown constant " + c->toString(ctx));
            return ast::GlobalState::defn_dynamic();

        } else if (ast::ConstantLit *scope = dynamic_cast<ast::ConstantLit *>(c->scope.get())) {
            auto resolved = resolveConstant(ctx, scope);
            if (!resolved.exists() || resolved == ast::GlobalState::defn_dynamic())
                return resolved;
            ast::SymbolRef result = resolved.info(ctx).findMember(c->cnst);
            if (!result.exists()) {
                ctx.state.errors.error(c->loc, ast::ErrorClass::StubConstant,
                                       "Stubbing out unknown constant " + c->toString(ctx));
                result = ast::GlobalState::defn_dynamic();
            }
            c->scope = make_unique<ast::Ident>(c->loc, resolved);

            return result;
        } else {
            ctx.state.errors.error(c->loc, ast::ErrorClass::DynamicConstant,
                                   "Dynamic constant references are unsupported " + c->toString(ctx));
            return ast::GlobalState::defn_dynamic();
        }
    }

    unique_ptr<ast::Expression> maybeResolve(ast::Context ctx, ast::Expression *expr) {
        if (ast::ConstantLit *cnst = dynamic_cast<ast::ConstantLit *>(expr)) {
            ast::SymbolRef resolved = resolveConstant(ctx, cnst);
            if (resolved.exists())
                return make_unique<ast::Ident>(expr->loc, resolved);
        }
        return nullptr;
    }

    shared_ptr<ast::Type> getResultType(ast::Context ctx, unique_ptr<ast::Expression> &expr) {
        shared_ptr<ast::Type> result;
        typecase(expr.get(),
                 [&](ast::Array *arr) {
                     vector<shared_ptr<ast::Type>> elems;
                     for (auto &el : arr->elems) {
                         elems.emplace_back(getResultType(ctx, el));
                     }
                     result = make_shared<ast::ArrayType>(elems);
                 },
                 [&](ast::Ident *i) {
                     if (i->symbol.info(ctx).isClass()) {
                         result = make_shared<ast::ClassType>(i->symbol);
                     } else {
                         ctx.state.errors.error(i->loc, ast::ErrorClass::InvalidTypeDeclaration,
                                                "Malformed type declaration. Not a class type {}" + i->toString(ctx));
                         result = ast::Types::dynamic();
                     }
                 },
                 [&](ast::Send *s) {
                     if (auto *recvi = dynamic_cast<ast::Ident *>(s->recv.get())) {
                         if (recvi->symbol != ast::GlobalState::defn_Opus_Types()) {
                             ctx.state.errors.error(recvi->loc, ast::ErrorClass::InvalidTypeDeclaration,
                                                    "Misformed type declaration. Unknown argument type type {}",
                                                    expr->toString(ctx));
                             result = ast::Types::dynamic();
                         } else {
                             if (s->fun == ast::Names::nilable()) {
                                 result = make_shared<ast::OrType>(getResultType(ctx, s->args[0]), ast::Types::nil());
                             } else if (s->fun == ast::Names::all()) {
                                 result = getResultType(ctx, s->args[0]);
                                 int i = 1;
                                 while (i < s->args.size()) {
                                     result = make_shared<ast::AndType>(result, getResultType(ctx, s->args[i]));
                                     i++;
                                 }
                             } else if (s->fun == ast::Names::any()) {
                                 result = getResultType(ctx, s->args[0]);
                                 int i = 1;
                                 while (i < s->args.size()) {
                                     result = make_shared<ast::OrType>(result, getResultType(ctx, s->args[i]));
                                     i++;
                                 }
                             } else {
                                 ctx.state.errors.error(s->loc, ast::ErrorClass::InvalidTypeDeclaration,
                                                        "Unsupported type combinator {}", s->fun.toString(ctx));
                                 result = ast::Types::dynamic();
                             }
                         }
                     } else {
                         ctx.state.errors.error(expr->loc, ast::ErrorClass::InvalidTypeDeclaration,
                                                "Misformed type declaration. Unknown type syntax {}",
                                                expr->toString(ctx));
                         result = ast::Types::dynamic();
                     }
                 });
        Error::check(result.get() != nullptr);
        return result;
    }

    void fillInInfoFromStandardMethod(ast::Context ctx, ast::Symbol &methoInfo,
                                      unique_ptr<ast::Send> &lastStandardMethod, int argsSize) {
        if (dynamic_cast<ast::Self *>(lastStandardMethod->recv.get()) == nullptr ||
            lastStandardMethod->block != nullptr) {
            ctx.state.errors.error(lastStandardMethod->loc, ast::ErrorClass::InvalidMethodSignature,
                                   "Misformed standard_method " + lastStandardMethod->toString(ctx));
        } else {
            for (auto &arg : lastStandardMethod->args) {
                if (auto *hash = dynamic_cast<ast::Hash *>(arg.get())) {
                    int i = 0;
                    for (unique_ptr<ast::Expression> &key : hash->keys) {
                        unique_ptr<ast::Expression> &value = hash->values[i];
                        if (auto *symbolLit = dynamic_cast<ast::SymbolLit *>(key.get())) {
                            if (symbolLit->name == ast::Names::returns()) {
                                // fill in return type
                                methoInfo.resultType = getResultType(ctx, value);
                                methoInfo.definitionLoc = value->loc;
                            } else {
                                auto fnd = find_if(
                                    methoInfo.arguments().begin(), methoInfo.arguments().end(),
                                    [&](ast::SymbolRef sym) -> bool { return sym.info(ctx).name == symbolLit->name; });
                                if (fnd == methoInfo.arguments().end()) {
                                    ctx.state.errors.error(key->loc, ast::ErrorClass::InvalidMethodSignature,
                                                           "Misformed standard_method. Unknown argument name type {}" +
                                                               key->toString(ctx));
                                } else {
                                    ast::SymbolRef arg = *fnd;
                                    arg.info(ctx).resultType = getResultType(ctx, value);
                                    arg.info(ctx).definitionLoc = key->loc;
                                }
                            }
                        } else {
                            ctx.state.errors.error(key->loc, ast::ErrorClass::InvalidMethodSignature,
                                                   "Misformed standard_method. Unknown key type {}" +
                                                       key->toString(ctx));
                        }
                    }
                }
            }
        }
    }

    void processDeclareVariables(ast::Context ctx, ast::Send *send) {
        if (dynamic_cast<ast::Self *>(send->recv.get()) == nullptr || send->block != nullptr) {
            ctx.state.errors.error(send->loc, ast::ErrorClass::InvalidDeclareVariables,
                                   "Malformed `declare_variables'");
            return;
        }

        if (send->args.size() != 1) {
            ctx.state.errors.error(send->loc, ast::ErrorClass::InvalidDeclareVariables,
                                   "Wrong number of arguments to `declare_variables'");
            return;
        }
        auto hash = dynamic_cast<ast::Hash *>(send->args.front().get());
        if (hash == nullptr) {
            ctx.state.errors.error(send->loc, ast::ErrorClass::InvalidDeclareVariables,
                                   "Malformed `declare_variables': Argument must be a hash");
            return;
        }
        for (int i = 0; i < hash->keys.size(); ++i) {
            auto &key = hash->keys[i];
            auto &value = hash->values[i];
            auto sym = dynamic_cast<ast::SymbolLit *>(key.get());
            if (sym == nullptr) {
                ctx.state.errors.error(key->loc, ast::ErrorClass::InvalidDeclareVariables,
                                       "`declare_variables': variable names must be symbols");
                continue;
            }

            auto typ = getResultType(ctx, value);
            ast::SymbolRef var;

            auto str = sym->name.toString(ctx);
            if (str.substr(0, 2) == "@@") {
                ast::Symbol &info = ctx.owner.info(ctx);
                var = info.findMember(sym->name);
                if (var.exists()) {
                    ctx.state.errors.error(key->loc, ast::ErrorClass::DuplicateVariableDeclaration,
                                           "Redeclaring variable `{}'", str);
                } else {
                    var = ctx.state.enterStaticFieldSymbol(ctx.owner, sym->name);
                }
            } else if (str.substr(0, 1) == "@") {
                ast::Symbol &info = ctx.owner.info(ctx);
                var = info.findMember(sym->name);
                if (var.exists()) {
                    ctx.state.errors.error(key->loc, ast::ErrorClass::DuplicateVariableDeclaration,
                                           "Redeclaring variable `{}'", str);
                } else {
                    var = ctx.state.enterFieldSymbol(ctx.owner, sym->name);
                }
            } else {
                ctx.state.errors.error(key->loc, ast::ErrorClass::InvalidDeclareVariables,
                                       "`declare_variables`: variables must start with @ or @@");
            }

            if (var.exists())
                var.info(ctx).resultType = typ;
        }
    }

public:
    ResolveWalk(ast::Context ctx) : nesting_(make_unique<Nesting>(nullptr, ctx.state.defn_root())) {}

    ast::ClassDef *preTransformClassDef(ast::Context ctx, ast::ClassDef *original) {
        nesting_ = make_unique<Nesting>(move(nesting_), original->symbol);
        return original;
    }
    ast::Statement *postTransformClassDef(ast::Context ctx, ast::ClassDef *original) {
        nesting_ = move(nesting_->parent);

        for (auto &ancst : original->ancestors) {
            if (auto resolved = maybeResolve(ctx, ancst.get()))
                ancst.swap(resolved);
        }
        ast::Symbol &info = original->symbol.info(ctx);
        if (original->ancestors.size() > 0) {
            ast::Symbol &info = original->symbol.info(ctx);
            for (auto &ancst : original->ancestors) {
                if (ast::Ident *id = dynamic_cast<ast::Ident *>(ancst.get())) {
                    info.argumentsOrMixins.emplace_back(id->symbol);
                    if (&ancst == &original->ancestors.front()) {
                        if (!info.superClass.exists() || info.superClass == ast::GlobalState::defn_object() ||
                            info.superClass == id->symbol) {
                            info.superClass = id->symbol;
                        } else {
                            ctx.state.errors.error(id->loc, ast::ErrorClass::RedefinitionOfParents,
                                                   "Class parents redefined for class {}",
                                                   original->symbol.info(ctx).name.toString(ctx));
                        }
                    }
                }
            }
        } else if (!info.superClass.exists() && original->symbol != ast::GlobalState::defn_Basic_Object()) {
            info.superClass = ast::GlobalState::defn_object();
            info.argumentsOrMixins.emplace_back(ast::GlobalState::defn_object());
        }

        unique_ptr<ast::Send> lastStandardMethod;
        for (auto &stat : original->rhs) {
            if (auto send = dynamic_cast<ast::Send *>(stat.get())) {
                if (send->fun == ast::Names::standardMethod()) {
                    lastStandardMethod.reset(send);
                    stat.release();
                } else if (send->fun == ast::Names::declareVariables()) {
                    processDeclareVariables(ctx.withOwner(original->symbol), send);
                }
            } else if (auto mdef = dynamic_cast<ast::MethodDef *>(stat.get())) {
                if (lastStandardMethod) {
                    ast::Symbol &methoInfo = mdef->symbol.info(ctx);
                    fillInInfoFromStandardMethod(ctx, methoInfo, lastStandardMethod, mdef->args.size());
                    lastStandardMethod.reset(nullptr);
                }
            }
        }

        auto toRemove = remove_if(original->rhs.begin(), original->rhs.end(),
                                  [](unique_ptr<ast::Statement> &stat) -> bool { return stat.get() == nullptr; });

        original->rhs.erase(toRemove, original->rhs.end());

        return original;
    }

    ast::Statement *postTransformConstantLit(ast::Context ctx, ast::ConstantLit *c) {
        ast::SymbolRef resolved = resolveConstant(ctx, c);
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
    ast::Statement *postTransformUnresolvedIdent(ast::Context ctx, ast::UnresolvedIdent *id) {
        ast::SymbolRef klass;

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

        ast::SymbolRef sym = klass.info(ctx).findMemberTransitive(ctx, id->name);
        if (!sym.exists()) {
            ctx.state.errors.error(id->loc, ast::ErrorClass::UndeclaredVariable, "Use of undeclared variable `{}'",
                                   id->name.toString(ctx));
            if (id->kind == ast::UnresolvedIdent::Class) {
                sym = ctx.state.enterStaticFieldSymbol(klass, id->name);
            } else {
                sym = ctx.state.enterFieldSymbol(klass, id->name);
            }
        }

        return new ast::Ident(id->loc, sym);
    };
};

unique_ptr<ast::Statement> Namer::resolve(ast::Context &ctx, unique_ptr<ast::Statement> tree) {
    ResolveWalk walk(ctx);
    tree = ast::TreeMap<ResolveWalk>::apply(ctx, walk, move(tree));
    ResolveVariablesWalk vars;
    tree = ast::TreeMap<ResolveVariablesWalk>::apply(ctx, vars, move(tree));
    return tree;
}
} // namespace namer
} // namespace ruby_typer
