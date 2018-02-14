#include "core/errors/resolver.h"
#include "ast/Trees.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/Names/resolver.h"
#include "core/core.h"
#include "resolver/resolver.h"
#include "resolver/type_syntax.h"

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
                ctx.state.error(c->loc, core::errors::Resolver::StubConstant, "Stubbing out unknown constant");
                result = ctx.state.enterClassSymbol(c->loc, nesting_.get()->scope, c->cnst);
                result.data(ctx).superClass = core::Symbols::StubClass();
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
                    ctx.state.error(c->loc, core::errors::Resolver::StubConstant, "Stubbing out unknown constant");
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

            auto typ = TypeSyntax::getResultType(ctx, value);
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

    void fillInInfoFromSig(core::Context ctx, core::SymbolRef method, ast::Send *send, bool isOverloaded) {
        auto exprLoc = send->loc;
        auto &methodInfo = method.data(ctx);

        auto sig = TypeSyntax::parseSig(ctx, send);

        if (!sig.seen.returns) {
            if (sig.seen.args || !(sig.seen.abstract || sig.seen.override_ || sig.seen.implementation ||
                                   sig.seen.overridable || sig.seen.abstract)) {
                ctx.state.error(exprLoc, core::errors::Resolver::InvalidMethodSignature,
                                "Malformed `sig`: No return type specified. Specify one with .returns()");
            }
        }

        methodInfo.resultType = sig.returns;

        for (auto it = methodInfo.arguments().begin(); it != methodInfo.arguments().end(); /* nothing */) {
            core::SymbolRef arg = *it;
            auto spec = find_if(sig.argTypes.begin(), sig.argTypes.end(),
                                [&](auto &spec) { return spec.name == arg.data(ctx).name; });
            if (spec != sig.argTypes.end()) {
                ENFORCE(spec->type != nullptr);
                arg.data(ctx).resultType = spec->type;
                arg.data(ctx).definitionLoc = spec->loc;
                sig.argTypes.erase(spec);
                ++it;
            } else if (isOverloaded) {
                it = methodInfo.arguments().erase(it);
            } else if (arg.data(ctx).resultType != nullptr) {
                ++it;
            } else {
                arg.data(ctx).resultType = core::Types::dynamic();
                if (sig.seen.args || sig.seen.returns) {
                    // Only error if we have any types
                    ctx.state.error(arg.data(ctx).definitionLoc, core::errors::Resolver::InvalidMethodSignature,
                                    "Malformed sig. Type not specified for argument {}",
                                    arg.data(ctx).name.toString(ctx));
                }
                ++it;
            }

            if (isOverloaded && arg.data(ctx).isKeyword()) {
                ctx.state.error(arg.data(ctx).definitionLoc, core::errors::Resolver::InvalidMethodSignature,
                                "Malformed sig. Overloaded functions cannot have keyword arguments:  {}",
                                arg.data(ctx).name.toString(ctx));
            }
        }

        for (auto spec : sig.argTypes) {
            ctx.state.error(spec.loc, core::errors::Resolver::InvalidMethodSignature, "Unknown argument name {}",
                            spec.name.toString(ctx));
        }
    }

    void processClassBody(core::Context ctx, ast::ClassDef *klass) {
        InlinedVector<unique_ptr<ast::Expression>, 1> lastSig;
        for (auto &stat : klass->rhs) {
            typecase(stat.get(),

                     [&](ast::Send *send) {
                         if (TypeSyntax::isSig(ctx, send)) {
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
            auto recv = ast::cast_tree<ast::Self>(send->recv.get());
            ENFORCE(recv);
            ENFORCE(send->fun == core::Names::typeMember());
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
                        data.resultType = TypeSyntax::getResultType(ctx, hash->values[i]);
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
        processClassBody(ctx.withOwner(original->symbol), original);
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
                auto type = TypeSyntax::getResultType(ctx, send->args[1]);
                return new ast::Cast(send->loc, type, move(expr), checked);
            }
            default:
                return send;
        }
    }
}; // namespace namer

class FlattenWalk {
private:
    bool isDefinition(core::Context ctx, const unique_ptr<ast::Expression> &what) {
        if (ast::isa_tree<ast::MethodDef>(what.get())) {
            return true;
        }
        if (ast::isa_tree<ast::ClassDef>(what.get())) {
            return true;
        }

        if (auto asgn = ast::cast_tree<ast::Assign>(what.get())) {
            return ast::isa_tree<ast::ConstantLit>(asgn->lhs.get());
        }
        return false;
    }

    unique_ptr<ast::Expression> extractClassInit(core::Context ctx, ast::ClassDef *klass) {
        ast::InsSeq::STATS_store inits;

        for (auto it = klass->rhs.begin(); it != klass->rhs.end(); /* nothing */) {
            if (isDefinition(ctx, *it)) {
                ++it;
                continue;
            }
            inits.emplace_back(move(*it));
            it = klass->rhs.erase(it);
        }

        if (inits.empty()) {
            return nullptr;
        }
        if (inits.size() == 1) {
            return move(inits.front());
        }
        return make_unique<ast::InsSeq>(klass->loc, move(inits), make_unique<ast::EmptyTree>(core::Loc::none()));
    }

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

        auto inits = extractClassInit(ctx, classDef);
        if (inits == nullptr) {
            return classDef;
        }

        auto nm = core::Names::staticInit();
        if (classDef->symbol == core::Symbols::root()) {
            // Every file may have its own top-level code, so uniqify the names.
            //
            // NOTE(nelhage): In general, we potentially need to do this for
            // every class, since Ruby allows reopening classes. However, since
            // pay-server bans that behavior, this should be OK here.
            nm = ctx.state.freshNameUnique(core::UniqueNameKind::Namer, nm, classDef->loc.file.id());
        }

        auto sym = ctx.state.enterMethodSymbol(inits->loc, classDef->symbol, nm);

        auto init = make_unique<ast::MethodDef>(inits->loc, sym, core::Names::staticInit(),
                                                ast::MethodDef::ARGS_store(), move(inits), true);
        classDef->rhs.emplace_back(move(init));

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
            ENFORCE(method.get() != nullptr);
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
