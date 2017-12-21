#include "core/errors/resolver.h"
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
            auto lookup = scope->scope.info(ctx).findMember(ctx, name);
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
                ctx.state.errors.error(c->loc, core::errors::Resolver::StubConstant, "Stubbing out unknown constant {}",
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
            core::SymbolRef result = resolved.info(ctx).findMember(ctx, c->cnst);
            if (!result.exists()) {
                if (resolved.info(ctx).resultType.get() == nullptr || !resolved.info(ctx).resultType->isDynamic()) {
                    ctx.state.errors.error(c->loc, core::errors::Resolver::StubConstant,
                                           "Stubbing out unknown constant {}", c->toString(ctx));
                }
                result = ctx.state.enterClassSymbol(c->loc, resolved, c->cnst);
                result.info(ctx).resultType = core::Types::dynamic();
            }

            return result;
        } else {
            ctx.state.errors.error(c->loc, core::errors::Resolver::DynamicConstant,
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

public:
    ResolveConstantsWalk(core::Context ctx) : nesting_(make_unique<Nesting>(nullptr, ctx.state.defn_root())) {}

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
        if (original->kind == ast::Module && info.mixins(ctx).empty()) {
            info.mixins(ctx).emplace_back(core::GlobalState::defn_BasicObject());
        }
        for (auto &ancst : original->ancestors) {
            ast::Ident *id = ast::cast_tree<ast::Ident>(ancst.get());
            if (id == nullptr || !id->symbol.info(ctx).isClass()) {
                ctx.state.errors.error(ancst->loc, core::errors::Resolver::DynamicSuperclass,
                                       "Superclasses and mixins must be statically resolved.");
                continue;
            }
            if (id->symbol == original->symbol || id->symbol.info(ctx).derivesFrom(ctx, original->symbol)) {
                ctx.state.errors.error(id->loc, core::errors::Resolver::CircularDependency,
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
                    ctx.state.errors.error(id->loc, core::errors::Resolver::RedefinitionOfParents,
                                           "Class parents redefined for class {}",
                                           original->symbol.info(ctx).name.toString(ctx));
                }
            } else {
                info.argumentsOrMixins.emplace_back(id->symbol);
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
        if (id == nullptr || !id->symbol.info(ctx).isStaticField()) {
            return asgn;
        }

        auto *rhs = ast::cast_tree<ast::Ident>(asgn->rhs.get());
        if (rhs == nullptr || !rhs->symbol.info(ctx).isClass()) {
            return asgn;
        }

        id->symbol.info(ctx).resultType = make_unique<core::ClassType>(rhs->symbol.info(ctx).singletonClass(ctx));
        return asgn;
    }

    unique_ptr<Nesting> nesting_;
};

class ResolveSignaturesWalk {
private:
    void processDeclareVariables(core::Context ctx, ast::Send *send) {
        if (send->block != nullptr) {
            ctx.state.errors.error(send->loc, core::errors::Resolver::InvalidDeclareVariables,
                                   "Malformed `declare_variables'");
            return;
        }

        if (send->args.size() != 1) {
            ctx.state.errors.error(send->loc, core::errors::Resolver::InvalidDeclareVariables,
                                   "Wrong number of arguments to `declare_variables'");
            return;
        }
        auto hash = ast::cast_tree<ast::Hash>(send->args.front().get());
        if (hash == nullptr) {
            ctx.state.errors.error(send->loc, core::errors::Resolver::InvalidDeclareVariables,
                                   "Malformed `declare_variables': Argument must be a hash");
            return;
        }
        for (int i = 0; i < hash->keys.size(); ++i) {
            auto &key = hash->keys[i];
            auto &value = hash->values[i];
            auto sym = ast::cast_tree<ast::SymbolLit>(key.get());
            if (sym == nullptr) {
                ctx.state.errors.error(key->loc, core::errors::Resolver::InvalidDeclareVariables,
                                       "`declare_variables': variable names must be symbols");
                continue;
            }

            auto typ = getResultType(ctx, value);
            core::SymbolRef var;

            auto str = sym->name.toString(ctx);
            if (str.substr(0, 2) == "@@") {
                core::Symbol &info = ctx.owner.info(ctx);
                var = info.findMember(ctx, sym->name);
                if (var.exists()) {
                    ctx.state.errors.error(key->loc, core::errors::Resolver::DuplicateVariableDeclaration,
                                           "Redeclaring variable `{}'", str);
                } else {
                    var = ctx.state.enterStaticFieldSymbol(sym->loc, ctx.owner, sym->name);
                }
            } else if (str.substr(0, 1) == "@") {
                core::Symbol &info = ctx.owner.info(ctx);
                var = info.findMember(ctx, sym->name);
                if (var.exists()) {
                    ctx.state.errors.error(key->loc, core::errors::Resolver::DuplicateVariableDeclaration,
                                           "Redeclaring variable `{}'", str);
                } else {
                    var = ctx.state.enterFieldSymbol(sym->loc, ctx.owner, sym->name);
                }
            } else {
                ctx.state.errors.error(key->loc, core::errors::Resolver::InvalidDeclareVariables,
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
                ctx.state.errors.error(arg->loc, core::errors::Resolver::InvalidAttr,
                                       "{} argument must be a string or symbol literal", send->fun.toString(ctx));
                continue;
            }

            core::NameRef varName = ctx.state.enterNameUTF8("@" + name.toString(ctx));
            core::SymbolRef sym = ctx.owner.info(ctx).findMemberTransitive(ctx, varName);
            if (!sym.exists()) {
                ctx.state.errors.error(arg->loc, core::errors::Resolver::UndeclaredVariable,
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
                     ctx.state.errors.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                            "Unsupported type literal");
                     result = core::Types::dynamic();
                 });
        Error::check(result.get() != nullptr);
        result->sanityCheck(ctx);
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
                    ctx.state.errors.error(i->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                           "Malformed type declaration. Not a class type {}", i->toString(ctx));
                    result = core::Types::dynamic();
                }
            },
            [&](ast::Send *s) {
                auto *recvi = ast::cast_tree<ast::Ident>(s->recv.get());
                if (recvi == nullptr) {
                    ctx.state.errors.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                           "Malformed type declaration. Unknown type syntax {}", expr->toString(ctx));
                    result = core::Types::dynamic();
                    return;
                }
                if (recvi->symbol != core::GlobalState::defn_Opus_Types()) {
                    // TODO(pay-server) remove this block
                    {
                        if (recvi->symbol == core::GlobalState::defn_Magic() && s->fun == core::Names::splat()) {
                            result = core::Types::bottom();
                            return;
                        }
                    }
                    ctx.state.errors.error(recvi->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                           "Malformed type declaration. Unknown argument type {}", expr->toString(ctx));
                    result = core::Types::dynamic();
                    return;
                }
                switch (s->fun._id) {
                    case core::Names::nilable()._id:
                        result = core::Types::buildOr(ctx, getResultType(ctx, s->args[0]), core::Types::nil());
                        break;
                    case core::Names::all()._id: {
                        result = getResultType(ctx, s->args[0]);
                        int i = 1;
                        while (i < s->args.size()) {
                            result = core::Types::buildAnd(ctx, result, getResultType(ctx, s->args[i]));
                            i++;
                        }
                        break;
                    }
                    case core::Names::any()._id: {
                        result = getResultType(ctx, s->args[0]);
                        int i = 1;
                        while (i < s->args.size()) {
                            result = core::Types::buildOr(ctx, result, getResultType(ctx, s->args[i]));
                            i++;
                        }
                        break;
                    }
                    case core::Names::enum_()._id: {
                        if (s->args.size() != 1) {
                            ctx.state.errors.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                                   "enum only takes a single argument");
                            result = core::Types::dynamic();
                            break;
                        }
                        auto arr = ast::cast_tree<ast::Array>(s->args[0].get());
                        if (arr == nullptr) {
                            // TODO(pay-server) unsilence this error and support enums from pay-server
                            {
                                result = core::Types::bottom();
                                break;
                            }
                            ctx.state.errors.error(
                                expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                "enum must be passed a literal array. e.g. enum([1,\"foo\",MyClass])");
                            result = core::Types::dynamic();
                            break;
                        }
                        if (arr->elems.empty()) {
                            ctx.state.errors.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                                   "enum([]) is invalid");
                            result = core::Types::dynamic();
                            break;
                        }
                        result = getResultLiteral(ctx, arr->elems[0]);
                        int i = 1;
                        while (i < arr->elems.size()) {
                            result = core::Types::buildOr(ctx, result, getResultLiteral(ctx, arr->elems[i]));
                            i++;
                        }
                        break;
                    }
                    case core::Names::interface()._id: {
                        if (s->args.size() != 1) {
                            ctx.state.errors.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                                   "Opus::Types.interface requires a single argument");
                            result = core::Types::dynamic();
                            break;
                        }
                        auto id = ast::cast_tree<ast::Ident>(s->args[0].get());
                        if (id == nullptr) {
                            ctx.state.errors.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                                   "Opus::Types.interface requires a class name as an argument");
                            result = core::Types::dynamic();
                            break;
                        }
                        auto sym = dealiasSym(ctx, id->symbol);
                        if (!sym.info(ctx).isClass()) {
                            ctx.state.errors.error(id->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                                   "Malformed type declaration. Not a class.");
                            result = core::Types::dynamic();
                            break;
                        }
                        result = make_shared<core::ClassType>(sym);
                        break;
                    }
                    case core::Names::untyped()._id:
                        result = core::Types::dynamic();
                        break;

                        /* TODO: array_of and hash_of accept arguments and
                           should instantiate generics, once we have those. */
                    case core::Names::arrayOf()._id:
                        result = core::Types::arrayClass();
                        break;
                    case core::Names::hashOf()._id:
                        result = core::Types::hashClass();
                        break;
                    case core::Names::noreturn()._id:
                        result = core::Types::bottom();
                        break;
                    default:
                        ctx.state.errors.error(s->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                               "Unsupported type combinator {}", s->fun.toString(ctx));
                        result = core::Types::dynamic();
                }
            },
            [&](ast::Expression *expr) {
                ctx.state.errors.error(expr->loc, core::errors::Resolver::InvalidTypeDeclaration,
                                       "Unsupported type syntax");
                result = core::Types::dynamic();
            });
        Error::check(result.get() != nullptr);
        result->sanityCheck(ctx);
        return result;
    }

    void fillInInfoFromStandardMethod(core::Context ctx, core::Symbol &methoInfo, ast::Send *lastStandardMethod,
                                      int argsSize) {
        if (ast::cast_tree<ast::Self>(lastStandardMethod->recv.get()) == nullptr ||
            lastStandardMethod->block != nullptr) {
            ctx.state.errors.error(lastStandardMethod->loc, core::errors::Resolver::InvalidMethodSignature,
                                   "Malformed {}: {} ", lastStandardMethod->fun.toString(ctx),
                                   lastStandardMethod->toString(ctx));
            return;
        }
        if (lastStandardMethod->args.empty()) {
            ctx.state.errors.error(lastStandardMethod->loc, core::errors::Resolver::InvalidMethodSignature,
                                   "No arguments passed to `{}'. Expected (arg_types, options)",
                                   lastStandardMethod->fun.toString(ctx));
            return;
        }
        if (lastStandardMethod->args.size() > 2) {
            ctx.state.errors.error(lastStandardMethod->loc, core::errors::Resolver::InvalidMethodSignature,
                                   "Wrong number of arguments for `{}'. Expected (arg_types, options)",
                                   lastStandardMethod->fun.toString(ctx));
            return;
        }
        // Both args must be Hash<Symbol>
        for (auto &arg : lastStandardMethod->args) {
            if (auto *hash = ast::cast_tree<ast::Hash>(arg.get())) {
                for (auto &key : hash->keys) {
                    if (ast::cast_tree<ast::SymbolLit>(key.get()) == nullptr) {
                        ctx.state.errors.error(arg->loc, core::errors::Resolver::InvalidMethodSignature,
                                               "Malformed {}. Keys must be symbol literals.",
                                               lastStandardMethod->fun.toString(ctx));
                        return;
                    }
                }
            } else {
                ctx.state.errors.error(arg->loc, core::errors::Resolver::InvalidMethodSignature,
                                       "Malformed {}. Expected a hash literal.", lastStandardMethod->fun.toString(ctx));
                return;
            }
        }

        ast::Hash *hash = ast::cast_tree<ast::Hash>(lastStandardMethod->args[0].get());
        Error::check(hash != nullptr);
        if (lastStandardMethod->args.size() == 2) {
            int i = 0;
            for (unique_ptr<ast::Expression> &key : hash->keys) {
                unique_ptr<ast::Expression> &value = hash->values[i++];
                if (auto *symbolLit = ast::cast_tree<ast::SymbolLit>(key.get())) {
                    auto fnd =
                        find_if(methoInfo.arguments().begin(), methoInfo.arguments().end(),
                                [&](core::SymbolRef sym) -> bool { return sym.info(ctx).name == symbolLit->name; });
                    if (fnd == methoInfo.arguments().end()) {
                        ctx.state.errors.error(key->loc, core::errors::Resolver::InvalidMethodSignature,
                                               "Malformed {}. Unknown argument name {}",
                                               lastStandardMethod->fun.toString(ctx), key->toString(ctx));
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
                        ctx.state.errors.error(key->loc, core::errors::Resolver::InvalidMethodSignature,
                                               "Malformed {}. Unknown argument name {}",
                                               lastStandardMethod->fun.toString(ctx), key->toString(ctx));
                }
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
                             case core::Names::abstractMethod()._id:
                             case core::Names::implementationMethod()._id:
                             case core::Names::overrideMethod()._id:
                             case core::Names::overridableMethod()._id:
                             case core::Names::overridableImplementationMethod()._id:
                                 if (lastStandardMethod) {
                                     ctx.state.errors.error(core::Reporter::ComplexError(
                                         lastStandardMethod->loc, core::errors::Resolver::InvalidMethodSignature,
                                         "Unused type annotation. No method def before next annotation.",
                                         core::Reporter::ErrorLine(mine->loc,
                                                                   "Type annotation that will be used instead.")));
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
                             counterInc("types.standard_method.count");
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
            ctx.state.errors.error(lastStandardMethod->loc, core::errors::Resolver::InvalidMethodSignature,
                                   "Malformed {}. No method def following it.",
                                   ast::cast_tree<ast::Send>(lastStandardMethod.get())->fun.toString(ctx));
        }

        auto toRemove = remove_if(klass->rhs.begin(), klass->rhs.end(),
                                  [](unique_ptr<ast::Expression> &stat) -> bool { return stat.get() == nullptr; });

        klass->rhs.erase(toRemove, klass->rhs.end());
    }

    core::SymbolRef dealiasSym(core::Context ctx, core::SymbolRef sym) {
        while (sym.info(ctx).isStaticField()) {
            auto *ct = dynamic_cast<core::ClassType *>(sym.info(ctx).resultType.get());
            if (ct == nullptr) {
                break;
            }
            auto klass = ct->symbol.info(ctx).attachedClass(ctx);
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
                         ctx.state.errors.error(
                             cast->loc, core::errors::Resolver::ConstantAssertType,
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
        if (id == nullptr || !id->symbol.info(ctx).isStaticField()) {
            return asgn;
        }

        if (id->symbol.info(ctx).resultType != nullptr) {
            return asgn;
        }

        id->symbol.info(ctx).resultType = resolveConstantType(ctx, asgn->rhs);

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
        if (id->symbol != core::GlobalState::defn_Opus_Types()) {
            return send;
        }
        bool checked = false;
        switch (send->fun._id) {
            case core::Names::assertType()._id:
                checked = true;
                /* fallthrough */
            case core::Names::cast()._id: {
                if (send->args.size() < 2) {
                    ctx.state.errors.error(send->loc, core::errors::Resolver::InvalidCast,
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
        Error::check(methodsStack.empty());
        Error::check(classes.empty());
        Error::check(classOrder.empty());
    }

    ast::ClassDef *preTransformClassDef(core::Context ctx, ast::ClassDef *classDef) {
        newMethodSet();
        classOrder.emplace_back(classDef->symbol);
        return classDef;
    }

    ast::Expression *postTransformClassDef(core::Context ctx, ast::ClassDef *classDef) {
        classDef->rhs = addMethods(ctx, move(classDef->rhs));
        classes.emplace_back(make_unique<ast::ClassDef>(classDef->loc, classDef->symbol, move(classDef->name),
                                                        move(classDef->ancestors), move(classDef->rhs),
                                                        classDef->kind));
        return new ast::EmptyTree(classDef->loc);
    };

    ast::MethodDef *preTransformMethodDef(core::Context ctx, ast::MethodDef *methodDef) {
        curMethodSet().order.emplace_back(methodDef->symbol);
        return methodDef;
    }

    ast::Expression *postTransformMethodDef(core::Context ctx, ast::MethodDef *methodDef) {
        curMethodSet().methods.emplace_back(make_unique<ast::MethodDef>(methodDef->loc, methodDef->symbol,
                                                                        move(methodDef->name), move(methodDef->args),
                                                                        move(methodDef->rhs), methodDef->isSelf));
        return new ast::EmptyTree(methodDef->loc);
    };

    std::unique_ptr<ast::Expression> addClasses(core::Context &ctx, std::unique_ptr<ast::Expression> tree) {
        if (classes.empty()) {
            Error::check(sortedClasses().size() == 0);
            return tree;
        }
        if (classes.size() == 1 && ast::cast_tree<ast::EmptyTree>(tree.get())) {
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
            Error::check(!!clas);
            insSeq->stats.emplace_back(move(clas));
        }
        return tree;
    }

    std::unique_ptr<ast::Expression> addMethods(core::Context &ctx, std::unique_ptr<ast::Expression> tree) {
        auto &methods = curMethodSet().methods;
        if (methods.empty()) {
            Error::check(popCurMethodDefs().size() == 0);
            return tree;
        }
        if (methods.size() == 1 && ast::cast_tree<ast::EmptyTree>(tree.get())) {
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
            Error::check(!!method);
            insSeq->stats.emplace_back(move(method));
        }
        return tree;
    }

private:
    vector<unique_ptr<ast::ClassDef>> sortedClasses() {
        vector<unique_ptr<ast::ClassDef>> ret;
        Error::check(classOrder.size() == classes.size());

        for (auto symbol : classOrder) {
            for (auto it = classes.begin(); it != classes.end(); ++it) {
                auto &classDef = *it;
                if (classDef->symbol == symbol) {
                    ret.emplace_back(move(classDef));
                    classes.erase(it);
                    break;
                }
            }
        }
        classOrder.clear();
        Error::check(classes.size() == 0);
        return ret;
    }

    ast::ClassDef::RHS_store addMethods(core::Context &ctx, ast::ClassDef::RHS_store rhs) {
        if (curMethodSet().methods.size() == 1 && rhs.size() == 1 && ast::cast_tree<ast::EmptyTree>(rhs[0].get())) {
            // It was only 1 method to begin with, put it back
            rhs.pop_back();
            rhs.emplace_back(move(popCurMethodDefs()[0]));
            return rhs;
        }
        for (auto &method : popCurMethodDefs()) {
            Error::check(!!method);
            rhs.emplace_back(move(method));
        }
        return rhs;
    }

    vector<unique_ptr<ast::MethodDef>> popCurMethodDefs() {
        vector<unique_ptr<ast::MethodDef>> ret;
        auto &methodStack = curMethodSet();
        methodStack.sanityCheck();
        auto &order = methodStack.order;
        auto &methods = methodStack.methods;

        for (auto symbol : order) {
            for (auto it = methods.begin(); it != methods.end(); ++it) {
                auto &methodDef = *it;
                if (methodDef->symbol == symbol) {
                    ret.emplace_back(move(methodDef));
                    methods.erase(it);
                    break;
                }
            }
        }
        order.clear();
        Error::check(methods.size() == 0);
        popCurMethodSet();
        return ret;
    };

    struct Methods {
        vector<unique_ptr<ast::MethodDef>> methods;
        vector<core::SymbolRef> order;
        void sanityCheck() {
            if (!debug_mode) {
                return;
            }
            Error::check(order.size() == methods.size(), order.size(), " != ", methods.size());
        }
        Methods() {}
    };
    void newMethodSet() {
        Methods methods;
        methodsStack.emplace_back(move(methods));
    }
    Methods &curMethodSet() {
        Error::check(methodsStack.size() > 0);
        return methodsStack.back();
    }
    void popCurMethodSet() {
        Error::check(methodsStack.size() > 0);
        methodsStack.pop_back();
    }

    vector<Methods> methodsStack;
    vector<unique_ptr<ast::ClassDef>> classes;
    vector<core::SymbolRef> classOrder;
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
            ctx.state.errors.error(id->loc, core::errors::Resolver::UndeclaredVariable,
                                   "Use of undeclared variable `{}'", id->name.toString(ctx));
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

namespace {
void finalizeResolution(core::GlobalState &gs) {
    for (int i = 1; i < gs.symbolsUsed(); ++i) {
        auto &info = core::SymbolRef(i).info(gs);
        if (!info.isClass() || info.superClass != core::GlobalState::defn_todo()) {
            continue;
        }

        auto attached = info.attachedClass(gs);
        bool isSingleton = attached.exists() && attached != core::GlobalState::defn_untyped();
        if (isSingleton) {
            if (attached == core::GlobalState::defn_BasicObject()) {
                info.superClass = core::GlobalState::defn_Class();
            } else if (!attached.info(gs).superClass.exists()) {
                info.superClass = core::GlobalState::defn_Module();
            } else {
                Error::check(attached.info(gs).superClass != core::GlobalState::defn_todo());
                info.superClass = attached.info(gs).superClass.info(gs).singletonClass(gs);
            }
        } else {
            info.superClass = core::GlobalState::defn_Object();
        }
    }
}
}; // namespace

class ResolveSanityCheckWalk {
public:
    ast::Expression *postTransformClassDef(core::Context ctx, ast::ClassDef *original) {
        Error::check(original->symbol != core::GlobalState::defn_todo());
        return original;
    }
    ast::Expression *postTransformMethodDef(core::Context ctx, ast::MethodDef *original) {
        Error::check(original->symbol != core::GlobalState::defn_todo());
        return original;
    }
    ast::Expression *postTransformConstDef(core::Context ctx, ast::ConstDef *original) {
        Error::check(original->symbol != core::GlobalState::defn_todo());
        return original;
    }
    ast::Expression *postTransformIdent(core::Context ctx, ast::Ident *original) {
        Error::check(original->symbol != core::GlobalState::defn_todo());
        return original;
    }
    ast::Expression *postTransformUnresolvedIdent(core::Context ctx, ast::UnresolvedIdent *original) {
        Error::raise("These should have all been removed");
    }
    ast::Expression *postTransformSelf(core::Context ctx, ast::Self *original) {
        Error::check(original->claz != core::GlobalState::defn_todo());
        return original;
    }
    ast::Expression *postTransformBlock(core::Context ctx, ast::Block *original) {
        Error::check(original->symbol != core::GlobalState::defn_todo());
        return original;
    }
};

std::vector<std::unique_ptr<ast::Expression>> Resolver::run(core::Context &ctx,
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

    finalizeResolution(ctx);

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
