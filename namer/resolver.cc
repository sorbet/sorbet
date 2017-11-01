#include "../ast/ast.h"
#include "ast/ast.h"
#include "namer/namer.h"

#include <algorithm> // find_if
#include <list>
#include <vector>

using namespace std;

namespace ruby_typer {
namespace namer {

class Resolver {
public:
    Resolver(unique_ptr<ast::Statement> tree) : tree_(move(tree)) {}
    void walk(ast::Context);

    unique_ptr<ast::Statement> tree_;
};

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
            } else {
                ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::StubConstant,
                                       "Stubbing out unknown constant " + c->toString(ctx));
                return ast::GlobalState::defn_dynamic();
            }
        } else if (ast::ConstantLit *scope = dynamic_cast<ast::ConstantLit *>(c->scope.get())) {
            auto resolved = resolveConstant(ctx, scope);
            if (!resolved.exists())
                return resolved;
            ast::SymbolRef result = resolved.info(ctx).findMember(c->cnst);
            if (!result.exists()) {
                ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::StubConstant,
                                       "Stubbing out unknown constant " + c->toString(ctx));
                result = ast::GlobalState::defn_dynamic();
            }
            c->scope = make_unique<ast::Ident>(resolved);

            return result;
        } else {
            ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::DynamicConstant,
                                   "Dynamic constant references are unsupported " + c->toString(ctx));
            return ast::GlobalState::defn_dynamic();
        }
    }

    unique_ptr<ast::Expression> maybeResolve(ast::Context ctx, ast::Expression *expr) {
        if (ast::ConstantLit *cnst = dynamic_cast<ast::ConstantLit *>(expr)) {
            ast::SymbolRef resolved = resolveConstant(ctx, cnst);
            if (resolved.exists())
                return make_unique<ast::Ident>(resolved);
        }
        return nullptr;
    }

    shared_ptr<ast::Type> getResultType(ast::Context ctx, std::unique_ptr<ast::Expression> &expr) {
        shared_ptr<ast::Type> result;
        typecase(expr.get(),
                 [&](ast::Ident *i) {
                     if (i->symbol.info(ctx).isClass()) {
                         result = make_shared<ast::ClassType>(i->symbol);
                     } else {
                         ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::InvalidMethodSignature,
                                                "Misformed standard_method. Not a class type {}" + i->toString(ctx));
                         result = ast::Types::dynamic();
                     }
                 },
                 [&](ast::Send *s) {
                     if (auto *recvi = dynamic_cast<ast::Ident *>(s->recv.get())) {
                         if (recvi->symbol != ast::GlobalState::defn_Opus_Types()) {
                             ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::InvalidMethodSignature,
                                                    "Misformed standard_method. Unknown argument type type {}",
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
                                 ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::InvalidMethodSignature,
                                                        "Unsupported type combinator {}", s->fun.toString(ctx));
                                 result = ast::Types::dynamic();
                             }
                         }
                     } else {
                         ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::InvalidMethodSignature,
                                                "Misformed standard_method. Unknown argument type type {}",
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
            ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::InvalidMethodSignature,
                                   "Misformed standard_method " + lastStandardMethod->toString(ctx));
        } else {
            for (auto &arg : lastStandardMethod->args) {
                if (auto *hash = dynamic_cast<ast::Hash *>(arg.get())) {
                    int i = 0;
                    for (std::unique_ptr<ast::Expression> &key : hash->keys) {
                        std::unique_ptr<ast::Expression> &value = hash->values[i];
                        if (auto *symbolLit = dynamic_cast<ast::SymbolLit *>(key.get())) {
                            if (symbolLit->name == ast::Names::returns()) {
                                // fill in return type
                                methoInfo.resultType = getResultType(ctx, value);
                            } else {
                                auto fnd = find_if(
                                    methoInfo.arguments().begin(), methoInfo.arguments().end(),
                                    [&](ast::SymbolRef sym) -> bool { return sym.info(ctx).name == symbolLit->name; });
                                if (fnd == methoInfo.arguments().end()) {
                                    ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::InvalidMethodSignature,
                                                           "Misformed standard_method. Unknown argument name type {}" +
                                                               key->toString(ctx));
                                } else {
                                    ast::SymbolRef arg = *fnd;
                                    arg.info(ctx).resultType = getResultType(ctx, value);
                                }
                            }
                        } else {
                            ctx.state.errors.error(ast::Loc::none(0), ast::ErrorClass::InvalidMethodSignature,
                                                   "Misformed standard_method. Unknown key type {}" +
                                                       key->toString(ctx));
                        }
                    }
                }
            }
        }
    }

public:
    ResolveWalk(ast::Context ctx, Resolver *resolv)
        : resolv_(resolv), nesting_(make_unique<Nesting>(nullptr, ctx.state.defn_root())) {}

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
        if (original->ancestors.size() > 0) {
            if (ast::Ident *id = dynamic_cast<ast::Ident *>(original->ancestors.front().get())) {
                ast::Symbol &info = original->symbol.info(ctx);
                info.superClass = id->symbol;
                info.argumentsOrMixins.emplace_back(id->symbol);
            }
        }
        unique_ptr<ast::Send> lastStandardMethod;
        for (auto &stat : original->rhs) {
            if (auto send = dynamic_cast<ast::Send *>(stat.get())) {
                if (send->fun == ast::Names::standardMethod()) {
                    lastStandardMethod.reset(send);
                    stat.release();
                }
            } else if (auto mdef = dynamic_cast<ast::MethodDef *>(stat.get())) {
                if (lastStandardMethod) {
                    ast::Symbol &methoInfo = mdef->symbol.info(ctx);
                    fillInInfoFromStandardMethod(ctx, methoInfo, lastStandardMethod, mdef->args.size());
                    lastStandardMethod.reset(nullptr);
                }
            }
        }

        auto toRemove = std::remove_if(original->rhs.begin(), original->rhs.end(),
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
        return new ast::Ident(resolved);
    }

    Resolver *resolv_;
    unique_ptr<Nesting> nesting_;
};

void Resolver::walk(ast::Context ctx) {
    ResolveWalk walk(ctx, this);
    tree_ = ast::TreeMap<ResolveWalk>::apply(ctx, walk, move(tree_));
}

unique_ptr<ast::Statement> Namer::resolve(ast::Context &ctx, unique_ptr<ast::Statement> tree) {
    Resolver resolv(move(tree));
    resolv.walk(ctx);
    return move(resolv.tree_);
}
} // namespace namer
} // namespace ruby_typer
