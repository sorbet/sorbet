#include "../ast/ast.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
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
            if (!result.exists()) {
                ctx.state.errors.error(c->loc, core::ErrorClass::StubConstant, "Stubbing out unknown constant {}",
                                       c->toString(ctx));
                result = ctx.state.enterClassSymbol(c->loc, nesting_.get()->scope, c->cnst);
                result.info(ctx).resultType = core::Types::dynamic();
            }
            return result;

        } else if (ast::ConstantLit *scope = ast::cast_tree<ast::ConstantLit>(c->scope.get())) {
            auto resolved = resolveConstant(ctx, scope);
            if (!resolved.exists() || resolved == core::GlobalState::defn_untyped()) {
                return resolved;
            }
            c->scope = make_unique<ast::Ident>(c->loc, resolved);
        }

        if (ast::Ident *id = ast::cast_tree<ast::Ident>(c->scope.get())) {
            core::SymbolRef resolved = id->symbol;
            core::SymbolRef result = resolved.info(ctx).findMember(c->cnst);
            if (!result.exists()) {
                if (resolved.info(ctx).resultType.get() == nullptr || !resolved.info(ctx).resultType->isDynamic()) {
                    ctx.state.errors.error(c->loc, core::ErrorClass::StubConstant, "Stubbing out unknown constant {}",
                                           c->toString(ctx));
                }
                result = ctx.state.enterClassSymbol(c->loc, resolved, c->cnst);
                result.info(ctx).resultType = core::Types::dynamic();
            }

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
            if (resolved.exists()) {
                return make_unique<ast::Ident>(expr->loc, resolved);
            }
        }
        return nullptr;
    }

    core::SymbolRef dealiasSym(core::Context ctx, core::SymbolRef sym) {
        while (sym.info(ctx).isStaticField()) {
            auto *ct = dynamic_cast<core::ClassType *>(sym.info(ctx).resultType.get());
            if (ct == nullptr)
                break;
            auto klass = ct->symbol.info(ctx).attachedClass(ctx);
            if (!klass.exists())
                break;

            sym = klass;
        }
        return sym;
    }

    shared_ptr<core::Type> getResultLiteral(core::Context ctx, unique_ptr<ast::Expression> &expr) {
        shared_ptr<core::Type> result;
        typecase(expr.get(), [&](ast::IntLit *lit) { result = make_shared<core::LiteralType>(lit->value); },
                 [&](ast::FloatLit *lit) { result = make_shared<core::LiteralType>(lit->value); },
                 [&](ast::BoolLit *lit) { result = make_shared<core::LiteralType>(lit->value); },
                 [&](ast::StringLit *lit) {
                     result = make_shared<core::LiteralType>(core::GlobalState::defn_String(), lit->value);
                 },
                 [&](ast::SymbolLit *lit) {
                     result = make_shared<core::LiteralType>(core::GlobalState::defn_Symbol(), lit->name);
                 },
                 [&](ast::Expression *expr) {
                     ctx.state.errors.error(expr->loc, core::ErrorClass::InvalidTypeDeclaration,
                                            "Unsupported type literal");
                     result = core::Types::dynamic();
                 });
        Error::check(result.get() != nullptr);
        return result;
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
                auto sym = dealiasSym(ctx, i->symbol);
                if (sym.info(ctx).isClass()) {
                    result = make_shared<core::ClassType>(sym);
                } else {
                    ctx.state.errors.error(i->loc, core::ErrorClass::InvalidTypeDeclaration,
                                           "Malformed type declaration. Not a class type {}", i->toString(ctx));
                    result = core::Types::dynamic();
                }
            },
            [&](ast::Send *s) {
                auto *recvi = ast::cast_tree<ast::Ident>(s->recv.get());
                if (recvi == nullptr) {
                    ctx.state.errors.error(expr->loc, core::ErrorClass::InvalidTypeDeclaration,
                                           "Malformed type declaration. Unknown type syntax {}", expr->toString(ctx));
                    result = core::Types::dynamic();
                    return;
                }
                if (recvi->symbol != core::GlobalState::defn_Opus_Types()) {
                    ctx.state.errors.error(recvi->loc, core::ErrorClass::InvalidTypeDeclaration,
                                           "Malformed type declaration. Unknown argument type type {}",
                                           expr->toString(ctx));
                    result = core::Types::dynamic();
                    return;
                }
                switch (s->fun._id) {
                    case core::Names::nilable()._id:
                        result = make_shared<core::OrType>(getResultType(ctx, s->args[0]), core::Types::nil());
                        break;
                    case core::Names::all()._id: {
                        result = getResultType(ctx, s->args[0]);
                        int i = 1;
                        while (i < s->args.size()) {
                            result = make_shared<core::AndType>(result, getResultType(ctx, s->args[i]));
                            i++;
                        }
                        break;
                    }
                    case core::Names::any()._id: {
                        result = getResultType(ctx, s->args[0]);
                        int i = 1;
                        while (i < s->args.size()) {
                            result = make_shared<core::OrType>(result, getResultType(ctx, s->args[i]));
                            i++;
                        }
                        break;
                    }
                    case core::Names::enum_()._id: {
                        if (s->args.size() != 1) {
                            ctx.state.errors.error(expr->loc, core::ErrorClass::InvalidTypeDeclaration,
                                                   "enum only takes a single argument");
                            result = core::Types::dynamic();
                            break;
                        }
                        auto arr = ast::cast_tree<ast::Array>(s->args[0].get());
                        if (!arr) {
                            // TODO(pay-server) unsilence this error and support enums from pay-server
                            {
                                result = core::Types::bottom();
                                break;
                            }
                            ctx.state.errors.error(
                                expr->loc, core::ErrorClass::InvalidTypeDeclaration,
                                "enum must be passed a literal array. e.g. enum([1,\"foo\",MyClass])");
                            result = core::Types::dynamic();
                            break;
                        }
                        if (arr->elems.empty()) {
                            ctx.state.errors.error(expr->loc, core::ErrorClass::InvalidTypeDeclaration,
                                                   "enum([]) is invalid");
                            result = core::Types::dynamic();
                            break;
                        }
                        result = getResultLiteral(ctx, arr->elems[0]);
                        int i = 1;
                        while (i < arr->elems.size()) {
                            result = make_shared<core::OrType>(result, getResultLiteral(ctx, arr->elems[i]));
                            i++;
                        }
                        break;
                    }
                    case core::Names::untyped()._id:
                        result = core::Types::dynamic();
                        break;

                        /* TODO: array_of and hash_of accept arguments and
                           should instantiate generics, once we have those. */
                    case core::Names::arrayOf()._id:
                        result = make_shared<core::ClassType>(core::GlobalState::defn_Array());
                        break;
                    case core::Names::hashOf()._id:
                        result = make_shared<core::ClassType>(core::GlobalState::defn_Hash());
                        break;
                    case core::Names::noreturn()._id:
                        result = core::Types::bottom();
                        break;
                    default:
                        ctx.state.errors.error(s->loc, core::ErrorClass::InvalidTypeDeclaration,
                                               "Unsupported type combinator {}", s->fun.toString(ctx));
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

    void fillInInfoFromStandardMethod(core::Context ctx, core::Symbol &methoInfo, ast::Send *lastStandardMethod,
                                      int argsSize) {
        if (ast::cast_tree<ast::Self>(lastStandardMethod->recv.get()) == nullptr ||
            lastStandardMethod->block != nullptr) {
            ctx.state.errors.error(lastStandardMethod->loc, core::ErrorClass::InvalidMethodSignature,
                                   "Malformed standard_method " + lastStandardMethod->toString(ctx));
            return;
        }
        if (lastStandardMethod->args.empty()) {
            ctx.state.errors.error(lastStandardMethod->loc, core::ErrorClass::InvalidMethodSignature,
                                   "No arguments passed to `standard_method'. Expected (arg_types, options)");
            return;
        }
        if (lastStandardMethod->args.size() > 2) {
            ctx.state.errors.error(lastStandardMethod->loc, core::ErrorClass::InvalidMethodSignature,
                                   "Wrong number of arguments for `standard_method'. Expected (arg_types, options)");
            return;
        }
        // Both args must be Hash<Symbol>
        for (auto &arg : lastStandardMethod->args) {
            if (auto *hash = ast::cast_tree<ast::Hash>(arg.get())) {
                for (auto &key : hash->keys) {
                    if (ast::cast_tree<ast::SymbolLit>(key.get()) == nullptr) {
                        ctx.state.errors.error(arg->loc, core::ErrorClass::InvalidMethodSignature,
                                               "Malformed standard_method. Keys must be symbol literals.");
                        return;
                    }
                }
            } else {
                ctx.state.errors.error(arg->loc, core::ErrorClass::InvalidMethodSignature,
                                       "Malformed standard_method. Expected a hash literal.");
                return;
            }
        }

        ast::Hash *hash = ast::cast_tree<ast::Hash>(lastStandardMethod->args[0].get());
        Error::check(hash);
        if (lastStandardMethod->args.size() == 2) {
            int i = 0;
            for (unique_ptr<ast::Expression> &key : hash->keys) {
                unique_ptr<ast::Expression> &value = hash->values[i++];
                if (auto *symbolLit = ast::cast_tree<ast::SymbolLit>(key.get())) {
                    auto fnd =
                        find_if(methoInfo.arguments().begin(), methoInfo.arguments().end(),
                                [&](core::SymbolRef sym) -> bool { return sym.info(ctx).name == symbolLit->name; });
                    if (fnd == methoInfo.arguments().end()) {
                        ctx.state.errors.error(key->loc, core::ErrorClass::InvalidMethodSignature,
                                               "Malformed standard_method. Unknown argument name type {}",
                                               key->toString(ctx));
                    } else {
                        core::SymbolRef arg = *fnd;
                        arg.info(ctx).resultType = getResultType(ctx, value);
                        arg.info(ctx).definitionLoc = key->loc;
                    }
                }
            }
            // We consumed the first hash, leave the next one for options
            hash = ast::cast_tree<ast::Hash>(lastStandardMethod->args[1].get());
        }

        int i = 0;
        for (unique_ptr<ast::Expression> &key : hash->keys) {
            unique_ptr<ast::Expression> &value = hash->values[i++];
            if (auto *symbolLit = ast::cast_tree<ast::SymbolLit>(key.get())) {
                switch (symbolLit->name._id) {
                    case core::Names::returns()._id:
                        // fill in return type
                        methoInfo.resultType = getResultType(ctx, value);
                        methoInfo.definitionLoc = value->loc;
                        break;
                    case core::Names::checked()._id:
                        break;
                    default:
                        ctx.state.errors.error(key->loc, core::ErrorClass::InvalidMethodSignature,
                                               "Malformed standard_method. Unknown argument name {}",
                                               key->toString(ctx));
                }
            }
        }
    }

    void processDeclareVariables(core::Context ctx, ast::Send *send) {
        if (send->block != nullptr) {
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

            if (var.exists()) {
                var.info(ctx).resultType = typ;
            }
        }
    }

    void defineAttr(core::Context ctx, ast::Send *send, bool r, bool w) {
        for (auto &arg : send->args) {
            core::NameRef name(0);
            if (auto *sym = ast::cast_tree<ast::SymbolLit>(arg.get())) {
                name = sym->name;
            } else if (auto *str = ast::cast_tree<ast::StringLit>(arg.get())) {
                name = str->value;
            } else {
                ctx.state.errors.error(arg->loc, core::ErrorClass::InvalidAttr,
                                       "{} argument must be a string or symbol literal", send->fun.toString(ctx));
                continue;
            }

            core::NameRef varName = ctx.state.enterNameUTF8("@" + name.toString(ctx));
            core::SymbolRef sym = ctx.owner.info(ctx).findMemberTransitive(ctx, varName);
            if (!sym.exists()) {
                ctx.state.errors.error(arg->loc, core::ErrorClass::UndeclaredVariable,
                                       "Accessor for undeclared variable `{}'", varName.toString(ctx));
                sym = ctx.state.enterFieldSymbol(arg->loc, ctx.owner, varName);
            }

            if (r) {
                core::SymbolRef meth = ctx.state.enterMethodSymbol(send->loc, ctx.owner, name);
                meth.info(ctx).resultType = sym.info(ctx).resultType;
            }
            if (w) {
                core::SymbolRef meth = ctx.state.enterMethodSymbol(send->loc, ctx.owner, name.addEq(ctx));
                core::SymbolRef arg0 = ctx.state.enterMethodArgumentSymbol(arg->loc, meth, core::Names::arg0());

                auto ty = sym.info(ctx).resultType;
                if (ty == nullptr) {
                    ty = core::Types::dynamic();
                }
                arg0.info(ctx).resultType = ty;
                meth.info(ctx).arguments().push_back(arg0);
                meth.info(ctx).resultType = ty;
            }
        }
    }

    void processClassBody(core::Context ctx, ast::ClassDef *klass) {
        unique_ptr<ast::Expression> lastStandardMethod;

        for (auto &stat : klass->rhs) {
            typecase(stat.get(),

                     [&](ast::Send *send) {
                         // Take ownership of the statement inside this block
                         unique_ptr<ast::Expression> mine;
                         swap(mine, stat);

                         if (ast::cast_tree<ast::Self>(send->recv.get()) == nullptr) {
                             swap(mine, stat);
                             return;
                         }
                         switch (send->fun._id) {
                             case core::Names::standardMethod()._id:
                                 if (lastStandardMethod) {
                                     ctx.state.errors.error(core::Reporter::ComplexError(
                                         lastStandardMethod->loc, core::ErrorClass::InvalidMethodSignature,
                                         "Unused standard_method. No method def before next standard_method.",
                                         core::Reporter::ErrorLine(mine->loc,
                                                                   "Next standard_method that is used instead.")));
                                 }
                                 swap(lastStandardMethod, mine);
                                 break;
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
                                 swap(mine, stat);
                                 return;
                         }
                         stat.reset(nullptr);
                     },

                     [&](ast::MethodDef *mdef) {
                         if (lastStandardMethod) {
                             core::Symbol &methoInfo = mdef->symbol.info(ctx);
                             fillInInfoFromStandardMethod(ctx, methoInfo,
                                                          ast::cast_tree<ast::Send>(lastStandardMethod.get()),
                                                          mdef->args.size());
                             lastStandardMethod.reset(nullptr);
                         }

                     },
                     [&](ast::ClassDef *cdef) {
                         // Leave in place
                     },

                     [&](ast::Assign *assgn) {
                         if (ast::Ident *id = ast::cast_tree<ast::Ident>(assgn->lhs.get())) {
                             if (id->symbol.info(ctx).name.name(ctx).kind == core::CONSTANT) {
                                 stat.reset(nullptr);
                             }
                         }
                     },

                     [&](ast::EmptyTree *e) { stat.reset(nullptr); },

                     [&](ast::Expression *e) {});
        }

        if (lastStandardMethod) {
            ctx.state.errors.error(lastStandardMethod->loc, core::ErrorClass::InvalidMethodSignature,
                                   "Malformed standard_method. No method def following it.");
        }

        auto toRemove = remove_if(klass->rhs.begin(), klass->rhs.end(),
                                  [](unique_ptr<ast::Expression> &stat) -> bool { return stat.get() == nullptr; });

        klass->rhs.erase(toRemove, klass->rhs.end());
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
            if (auto resolved = maybeResolve(ctx, ancst.get())) {
                ancst.swap(resolved);
            }
        }
        core::Symbol &info = original->symbol.info(ctx);
        for (auto &ancst : original->ancestors) {
            ast::Ident *id = ast::cast_tree<ast::Ident>(ancst.get());
            if (id == nullptr || !id->symbol.info(ctx).isClass()) {
                ctx.state.errors.error(ancst->loc, core::ErrorClass::DynamicSuperclass,
                                       "Superclasses and mixins must be statically resolved.");
                continue;
            }
            if (id->symbol.info(ctx).derivesFrom(ctx, original->symbol)) {
                ctx.state.errors.error(id->loc, core::ErrorClass::CircularDependency,
                                       "Circular dependency: {} and {} are declared as parents of each other",
                                       original->symbol.info(ctx).name.toString(ctx),
                                       id->symbol.info(ctx).name.toString(ctx));
                continue;
            }

            if (original->kind == ast::Class && &ancst == &original->ancestors.front()) {
                if (id->symbol == core::GlobalState::defn_todo()) {
                    continue;
                }
                if (!info.superClass.exists() || info.superClass == core::GlobalState::defn_todo() ||
                    info.superClass == id->symbol) {
                    info.superClass = id->symbol;
                } else {
                    ctx.state.errors.error(id->loc, core::ErrorClass::RedefinitionOfParents,
                                           "Class parents redefined for class {}",
                                           original->symbol.info(ctx).name.toString(ctx));
                }
            } else {
                info.argumentsOrMixins.emplace_back(id->symbol);
            }
        }

        processClassBody(ctx, original);

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
        if (id == nullptr || !id->symbol.info(ctx).isStaticField())
            return asgn;
        auto *rhs = ast::cast_tree<ast::Ident>(asgn->rhs.get());
        if (rhs == nullptr || !rhs->symbol.info(ctx).isClass())
            return asgn;

        id->symbol.info(ctx).resultType = make_unique<core::ClassType>(rhs->symbol.info(ctx).singletonClass(ctx));
        return asgn;
    }

    unique_ptr<Nesting> nesting_;
}; // namespace namer

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

void Resolver::finalize(core::GlobalState &gs) {
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto &info = core::SymbolRef(i).info(gs);
        if (!info.isClass()) {
            continue;
        }
        if (info.superClass == core::GlobalState::defn_todo()) {
            info.superClass = core::GlobalState::defn_Object();
        }
    }
}

} // namespace namer
} // namespace ruby_typer
