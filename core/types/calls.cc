#include "common/common.h"
#include "core/Names/core.h"
#include "core/Types.h"
#include "core/errors/infer.h"
#include <algorithm> // find_if, sort
#include <unordered_set>

template class std::vector<ruby_typer::core::SymbolRef>;
using namespace ruby_typer;
using namespace core;
using namespace std;

shared_ptr<Type> ProxyType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                                         shared_ptr<Type> fullType, shared_ptr<Type> *block) {
    categoryCounterInc("dispatch_call", "proxytype");
    return underlying->dispatchCall(ctx, name, callLoc, args, underlying, fullType, block);
}

shared_ptr<Type> ProxyType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return underlying->getCallArgumentType(ctx, name, i);
}

shared_ptr<Type> OrType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                      vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                      shared_ptr<Type> *block) {
    categoryCounterInc("dispatch_call", "ortype");
    shared_ptr<Type> lblock, rblock;
    auto leftRet = left->dispatchCall(ctx, name, callLoc, args, left, fullType, block == nullptr ? nullptr : &lblock);
    auto rightRet =
        right->dispatchCall(ctx, name, callLoc, args, right, fullType, block == nullptr ? nullptr : &rblock);
    if (block != nullptr) {
        if (lblock == nullptr && rblock == nullptr) {
            *block = Types::dynamic();
        } else if (lblock == nullptr) {
            *block = rblock;
        } else if (rblock == nullptr) {
            *block = lblock;
        } else {
            *block = Types::glb(ctx, lblock, rblock);
        }
    }
    return Types::lub(ctx, leftRet, rightRet);
}

shared_ptr<Type> OrType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return left->getCallArgumentType(ctx, name, i); // TODO: should glb with right
}

shared_ptr<Type> AppliedType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                           vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                                           shared_ptr<Type> fullType, shared_ptr<Type> *block) {
    categoryCounterInc("dispatch_call", "appliedType");
    ClassType ct(this->klass);
    return ct.dispatchCallWithTargs(ctx, name, callLoc, args, fullType, this->targs, block);
}

shared_ptr<Type> TypeVar::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                       vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                                       shared_ptr<Type> fullType, shared_ptr<Type> *block) {
    ENFORCE(isInstantiated);
    return instantiation->dispatchCall(ctx, name, callLoc, args, instantiation, fullType, block);
}

shared_ptr<Type> AndType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                       vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                                       shared_ptr<Type> fullType, shared_ptr<Type> *block) {
    categoryCounterInc("dispatch_call", "andtype");
    // "AppliedType {\n      klass = ::<constant:Hash>\n      targs = [\n        0 = untyped\n        0 = untyped\n
    // ]\n    } & BasicObject" this type should not have been constructed.
    // TODO: was constructed for lib/db/model/mixins/ipm_derived.rb ,
    // lib/db/model/middleware/enforce_durable_write_middleware.rb fail silently.

    return this->left->dispatchCall(ctx, name, callLoc, args, this->left, fullType, block);
}

shared_ptr<Type> AndType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return Types::lub(ctx, left->getCallArgumentType(ctx, name, i), right->getCallArgumentType(ctx, name, i));
}

shared_ptr<Type> ShapeType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                                         shared_ptr<Type> fullType, shared_ptr<Type> *block) {
    categoryCounterInc("dispatch_call", "shapetype");
    switch (fun._id) {
        case Names::freeze()._id: {
            return selfRef;
        }
        default:
            break;
    }
    return ProxyType::dispatchCall(ctx, fun, callLoc, args, selfRef, fullType, block);
}

std::shared_ptr<Type> TupleType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                              std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                              shared_ptr<Type> fullType, std::shared_ptr<Type> *block) {
    categoryCounterInc("dispatch_call", "tupletype");
    switch (fun._id) {
        case Names::freeze()._id: {
            return selfRef;
        }
        default:
            break;
    }
    return ProxyType::dispatchCall(ctx, fun, callLoc, args, selfRef, fullType, block);
}

shared_ptr<Type> MagicType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                                         shared_ptr<Type> fullType, shared_ptr<Type> *block) {
    categoryCounterInc("dispatch_call", "magictype");
    switch (fun._id) {
        case Names::buildHash()._id: {
            ENFORCE(args.size() % 2 == 0);

            vector<shared_ptr<LiteralType>> keys;
            vector<shared_ptr<Type>> values;
            for (int i = 0; i < args.size(); i += 2) {
                auto *key = cast_type<LiteralType>(args[i].type.get());
                if (key == nullptr) {
                    return core::Types::hashOfUntyped();
                }

                keys.push_back(shared_ptr<LiteralType>(args[i].type, key));
                values.push_back(args[i + 1].type);
            }
            return make_unique<ShapeType>(keys, values);
        }

        case Names::buildArray()._id: {
            vector<shared_ptr<Type>> elems;
            for (auto &elem : args) {
                elems.push_back(elem.type);
            }
            return make_unique<TupleType>(elems);
        }
        default:
            return ProxyType::dispatchCall(ctx, fun, callLoc, args, selfRef, fullType, block);
    }
}

namespace {
bool isSetter(core::Context ctx, core::NameRef fun) {
    if (fun.data(ctx).kind != NameKind::UTF8) {
        return false;
    }
    return fun.data(ctx).raw.utf8.back() == '=';
}

void matchArgType(core::Context ctx, core::Loc callLoc, core::SymbolRef inClass, core::NameRef fun,
                  TypeAndOrigins &argTpe, const core::Symbol &argSym, shared_ptr<core::Type> &fullType,
                  vector<shared_ptr<Type>> &targs, bool mayBeSetter = false) {
    shared_ptr<Type> expectedType = core::Types::resultTypeAsSeenFrom(ctx, argSym.ref(ctx), inClass, targs);
    if (!expectedType) {
        expectedType = Types::dynamic();
    }

    if (Types::isSubType(ctx, argTpe.type, expectedType)) {
        return;
    }
    if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::MethodArgumentMismatch)) {
        if (mayBeSetter && isSetter(ctx, fun)) {
            e.setHeader("Assigning a value to `{}` that does not match expected type `{}`", argSym.name.toString(ctx),
                        expectedType->show(ctx));
        } else {
            e.setHeader("Expression passed as an argument `{}` to method `{}` does not match expected type",
                        argSym.name.toString(ctx), fun.toString(ctx), expectedType->show(ctx));
            e.addErrorSection(core::ErrorSection({
                core::ErrorLine::from(argSym.definitionLoc, "Method `{}` has specified type of argument `{}` as `{}`",
                                      argSym.owner.data(ctx).name.toString(ctx), argSym.name.toString(ctx),
                                      expectedType->show(ctx)),
            }));
        }
        e.addErrorSection(core::ErrorSection("Got " + argTpe.type->show(ctx) + " originating from:",
                                             argTpe.origins2Explanations(ctx)));
    }
}

void missingArg(Context ctx, Loc callLoc, core::NameRef method, SymbolRef arg) {
    if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::MethodArgumentCountMismatch)) {
        e.setHeader("Missing required keyword argument `{}` for method `{}`.", arg.data(ctx).name.toString(ctx),
                    method.toString(ctx));
    }
}
}; // namespace

int getArity(core::Context ctx, core::SymbolRef method) {
    int arity = method.data(ctx).arguments().size();
    if (arity == 0) {
        return 0;
    }
    if (method.data(ctx).arguments().back().data(ctx).isBlockArgument()) {
        --arity;
    }
    return arity;
}

// Guess overload. The way we guess is only arity based - we will return the overload that has the smallest number of
// arguments that is >= args.size()
core::SymbolRef guessOverload(core::Context ctx, core::SymbolRef inClass, core::SymbolRef primary,
                              vector<TypeAndOrigins> &args, shared_ptr<Type> fullType,
                              std::vector<std::shared_ptr<Type>> &targs, bool hasBlock) {
    counterInc("calls.overloaded_invocations");
    ENFORCE(ctx.permitOverloadDefinitions(), "overload not permitted here");
    core::SymbolRef fallback = primary;
    vector<core::SymbolRef> allCandidates;

    allCandidates.push_back(primary);
    { // create candidates and sort them by number of arguments(stable by symbol id)
        int i = 0;
        core::SymbolRef current = primary;
        while (current.data(ctx).isOverloaded()) {
            i++;
            core::NameRef overloadName =
                ctx.state.getNameUnique(core::UniqueNameKind::Overload, primary.data(ctx).name, i);
            core::SymbolRef overload = primary.data(ctx).owner.data(ctx).findMember(ctx, overloadName);
            if (!overload.exists()) {
                Error::raise("Corruption of overloads?");
            } else {
                allCandidates.push_back(overload);
                current = overload;
            }
        }

        sort(allCandidates.begin(), allCandidates.end(), [&](core::SymbolRef s1, core::SymbolRef s2) -> bool {
            if (getArity(ctx, s1) < getArity(ctx, s2)) {
                return true;
            }
            if (getArity(ctx, s1) == getArity(ctx, s2)) {
                return s1._id < s2._id;
            }
            return false;
        });
    }

    vector<core::SymbolRef> leftCandidates = allCandidates;

    {
        // Lets see if we can filter them out using arguments.
        int i = -1;
        for (auto &arg : args) {
            i++;
            for (auto it = leftCandidates.begin(); it != leftCandidates.end(); /* nothing*/) {
                core::SymbolRef candidate = *it;
                if (i >= getArity(ctx, candidate)) {
                    it = leftCandidates.erase(it);
                    continue;
                }

                auto argType =
                    core::Types::resultTypeAsSeenFrom(ctx, candidate.data(ctx).arguments()[i], inClass, targs);
                if (argType->isFullyDefined() && !Types::isSubType(ctx, arg.type, argType)) {
                    it = leftCandidates.erase(it);
                    continue;
                }
                ++it;
            }
        }
    }
    if (leftCandidates.empty()) {
        leftCandidates = allCandidates;
    } else {
        fallback = leftCandidates[0];
    }

    { // keep only candidates that have a block iff we are passing one
        for (auto it = leftCandidates.begin(); it != leftCandidates.end(); /* nothing*/) {
            core::SymbolRef candidate = *it;
            auto args = candidate.data(ctx).argumentsOrMixins;
            if (args.empty()) {
                if (hasBlock) {
                    it = leftCandidates.erase(it);
                    continue;
                }
            } else {
                auto &lastArg = args.back().data(ctx);
                if (lastArg.isBlockArgument() != hasBlock) {
                    it = leftCandidates.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

    { // keep only candidates with closest arity
        struct Comp {
            core::Context ctx;

            bool operator()(core::SymbolRef s, int i) const {
                return getArity(ctx, s) < i;
            }
            bool operator()(int i, core::SymbolRef s) const {
                return i < getArity(ctx, s);
            }
            Comp(core::Context ctx) : ctx(ctx){};
        } cmp(ctx);

        auto er = std::equal_range(leftCandidates.begin(), leftCandidates.end(), args.size(), cmp);
        if (er.first != leftCandidates.end()) {
            leftCandidates.erase(leftCandidates.begin(), er.first);
        }
    }

    if (!leftCandidates.empty()) {
        return leftCandidates[0];
    }
    return fallback;
}

shared_ptr<Type> unwrapType(Context ctx, Loc loc, shared_ptr<Type> tp) {
    if (auto *tc = cast_type<MetaType>(tp.get())) {
        return tc->wrapped;
    }
    if (ClassType *classType = cast_type<ClassType>(tp.get())) {
        SymbolRef attachedClass = classType->symbol.data(ctx).attachedClass(ctx);
        if (!attachedClass.exists()) {
            if (auto e = ctx.state.beginError(loc, errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
            }
            return Types::dynamic();
        }

        return attachedClass.data(ctx).externalType(ctx);
    }
    return tp;
}

// This method handles a number of special-case methods that are implemented as
// intrinsics in C++. If it recognizes the method call, it handles it and
// returns a type; otherwise, it returns `nullptr.
std::shared_ptr<Type> ClassType::dispatchCallIntrinsic(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                                       std::vector<TypeAndOrigins> &args,
                                                       std::shared_ptr<Type> fullType,
                                                       std::vector<std::shared_ptr<Type>> &targs,
                                                       shared_ptr<Type> *block) {
    switch (name._id) {
        case core::Names::new_()._id: {
            auto attachedClass = this->symbol.data(ctx).attachedClass(ctx);
            if (!attachedClass.exists()) {
                // `foo.new(...)`, but foo isn't a Class
                return nullptr;
            }

            auto instanceTy = attachedClass.data(ctx).externalType(ctx);
            instanceTy->dispatchCall(ctx, core::Names::initialize(), callLoc, args, instanceTy, instanceTy, block);
            return instanceTy;
        }

        case core::Names::initialize()._id: {
            // Default initialize() implementation if the class doesn't provide
            // one in userland
            if (!args.empty()) {
                if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Wrong number of arguments for constructor. Expected: 0, found: `{}`", args.size());
                }
            }
            return core::Types::dynamic();
        }

        case core::Names::squareBrackets()._id: {
            core::SymbolRef attachedClass;

            attachedClass = this->symbol.data(ctx).attachedClass(ctx);

            if (!attachedClass.exists()) {
                return nullptr;
            }

            if (attachedClass == core::Symbols::T_Array()) {
                attachedClass = core::Symbols::Array();
            } else if (attachedClass == core::Symbols::T_Hash()) {
                attachedClass = core::Symbols::Hash();
            } else if (attachedClass == core::Symbols::T_Enumerable()) {
                attachedClass = core::Symbols::Enumerable();
            } else if (attachedClass == core::Symbols::T_Range()) {
                attachedClass = core::Symbols::Range();
            } else if (attachedClass == core::Symbols::T_Set()) {
                attachedClass = core::Symbols::Set();
            }

            auto arity = attachedClass.data(ctx).typeArity(ctx);
            if (attachedClass == core::Symbols::Hash()) {
                arity = 2;
            }
            if (attachedClass.data(ctx).typeMembers().empty()) {
                return nullptr;
            }

            if (args.size() != arity) {
                if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::GenericArgumentCountMismatch)) {
                    e.setHeader("Wrong number of type parameters for `{}`. Expected `{}`, got `{}`",
                                attachedClass.data(ctx).show(ctx), arity, args.size());
                }
            }

            vector<shared_ptr<core::Type>> targs;
            auto it = args.begin();
            int i = -1;
            for (auto mem : attachedClass.data(ctx).typeMembers()) {
                ++i;
                if (mem.data(ctx).isFixed()) {
                    targs.emplace_back(mem.data(ctx).resultType);
                } else if (it != args.end()) {
                    targs.emplace_back(unwrapType(ctx, it->origins[0], it->type));
                    ++it;
                } else if (attachedClass == core::Symbols::Hash() && i == 2) {
                    auto tupleArgs = targs;
                    targs.emplace_back(make_shared<TupleType>(tupleArgs));
                } else {
                    targs.emplace_back(core::Types::dynamic());
                }
            }

            return make_shared<core::MetaType>(make_shared<core::AppliedType>(attachedClass, targs));
        }

        case core::Names::untyped()._id:
        case core::Names::any()._id:
        case core::Names::all()._id:
            if (this->symbol != core::Symbols::T()) {
                return nullptr;
            }
            if (name == core::Names::untyped()) {
                return Types::dynamic();
            } else {
                shared_ptr<Type> res = (name == core::Names::all()) ? core::Types::top() : core::Types::bottom();
                for (auto &arg : args) {
                    auto ty = unwrapType(ctx, arg.origins[0], arg.type);
                    if (name == core::Names::any()) {
                        res = Types::buildOr(ctx, res, ty);
                    } else {
                        res = Types::buildAnd(ctx, res, ty);
                    }
                }

                return make_shared<core::MetaType>(res);
            }

        case core::Names::class_()._id:
        case core::Names::singletonClass()._id: {
            /*
             * x : Integer
             * =>
             * x.class = Integer
             * x.class : Integer.singleton_class
             */
            auto singleton = this->symbol.data(ctx).lookupSingletonClass(ctx);
            if (singleton.exists()) {
                return make_shared<ClassType>(singleton);
            }
            return core::Types::classClass();
        }
        default:
            return nullptr;
    }
}

// This implements Ruby's argument matching logic (assigning values passed to a
// method call to formal parameters of the method).
//
// Known incompleteness or inconsistencies with Ruby:
//  - Missing coercion to keyword arguments via `#to_hash`
//  - We never allow a non-shaped Hash to satisfy keyword arguments;
//    We should, at a minimum, probably allow one to satisfy an **kwargs : dynamic
//    (with a subtype check on the key type, once we have generics)
shared_ptr<Type> ClassType::dispatchCallWithTargs(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                                  vector<TypeAndOrigins> &args, shared_ptr<Type> fullType,
                                                  vector<shared_ptr<Type>> &targs, shared_ptr<Type> *block) {
    categoryCounterInc("dispatch_call", "classtype");
    if (isDynamic()) {
        return Types::dynamic();
    }
    core::SymbolRef mayBeOverloaded = this->symbol.data(ctx).findMemberTransitive(ctx, fun);

    if (!mayBeOverloaded.exists()) {
        auto special = dispatchCallIntrinsic(ctx, fun, callLoc, args, fullType, targs, block);
        if (special != nullptr) {
            return special;
        }

        string maybeComponent;
        core::Symbol::FuzzySearchResult alternative;

        if (fullType.get() != this) {
            maybeComponent = " component of " + fullType->show(ctx);
        } else {
            alternative = this->symbol.data(ctx).findMemberFuzzyMatch(ctx, fun);
        }

        if (alternative.symbol.exists()) {
            if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::UnknownMethod)) {
                e.setHeader("Method `{}` does not exist on `{}`", fun.data(ctx).toString(ctx), this->show(ctx));
                e.addErrorSection(core::ErrorSection({
                    core::ErrorLine::from(alternative.symbol.data(ctx).definitionLoc, "Did you mean: `{}`?",
                                          alternative.name.toString(ctx)),
                }));
            }
        } else {
            if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::UnknownMethod)) {
                e.setHeader("Method `{}` does not exist on `{}`{}", fun.data(ctx).toString(ctx), this->show(ctx),
                            maybeComponent);
            }
        }
        return Types::dynamic();
    }

    core::SymbolRef method = mayBeOverloaded.data(ctx).isOverloaded()
                                 ? guessOverload(ctx.withOwner(mayBeOverloaded), this->symbol, mayBeOverloaded, args,
                                                 fullType, targs, block != nullptr)
                                 : mayBeOverloaded;

    const core::Symbol &data = method.data(ctx);

    bool hasKwargs = std::any_of(data.arguments().begin(), data.arguments().end(),
                                 [&ctx](core::SymbolRef arg) { return arg.data(ctx).isKeyword(); });

    auto pit = data.arguments().begin();
    auto pend = data.arguments().end();

    if (pit != pend && (pend - 1)->data(ctx).isBlockArgument()) {
        --pend;
    }

    auto ait = args.begin();
    auto aend = args.end();

    while (pit != pend && ait != aend) {
        const core::Symbol &spec = pit->data(ctx);
        auto &arg = *ait;
        if (spec.isKeyword()) {
            break;
        }
        if (ait + 1 == aend && hasKwargs && arg.type->derivesFrom(ctx, core::Symbols::Hash()) &&
            (spec.isOptional() || spec.isRepeated())) {
            break;
        }

        matchArgType(ctx, callLoc, this->symbol, fun, arg, spec, fullType, targs, args.size() == 1);

        if (!spec.isRepeated()) {
            ++pit;
        }
        ++ait;
    }

    if (pit != pend) {
        if (!(pit->data(ctx).isKeyword() || pit->data(ctx).isOptional() || pit->data(ctx).isRepeated() ||
              pit->data(ctx).isBlockArgument())) {
            if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::MethodArgumentCountMismatch)) {
                e.setHeader("Not enough arguments provided for method `{}`. Expected: `{}`, provided: `{}`",
                            fun.toString(ctx),
                            // TODO(nelhage): report actual counts of required arguments,
                            // and account for keyword arguments
                            data.arguments().size(),
                            args.size()); // TODO: should use position and print the source tree, not the cfg one.
            }
        }
    }

    if (hasKwargs && ait != aend) {
        std::unordered_set<NameRef> consumed;
        auto &hashArg = *(aend - 1);

        // find keyword arguments and advance `pend` before them; We'll walk
        // `kwit` ahead below
        auto kwit = pit;
        while (!kwit->data(ctx).isKeyword()) {
            kwit++;
        }
        pend = kwit;

        if (hashArg.type->isDynamic()) {
            // Allow an untyped arg to satisfy all kwargs
            --aend;
        } else if (ShapeType *hash = cast_type<ShapeType>(hashArg.type.get())) {
            --aend;

            while (kwit != data.arguments().end()) {
                const core::Symbol &spec = kwit->data(ctx);
                if (spec.isBlockArgument()) {
                    break;
                } else if (spec.isRepeated()) {
                    for (auto it = hash->keys.begin(); it != hash->keys.end(); ++it) {
                        auto key = *it;
                        SymbolRef klass = cast_type<ClassType>(key->underlying.get())->symbol;
                        if (klass != core::Symbols::Symbol()) {
                            continue;
                        }

                        NameRef arg(ctx.state, key->value);
                        if (consumed.find(NameRef(ctx.state, key->value)) != consumed.end()) {
                            continue;
                        }
                        consumed.insert(arg);

                        TypeAndOrigins tpe;
                        tpe.origins = args.back().origins;
                        tpe.type = hash->values[it - hash->keys.begin()];
                        matchArgType(ctx, callLoc, this->symbol, fun, tpe, spec, fullType, targs);
                    }
                    break;
                }
                ++kwit;

                auto arg = find_if(hash->keys.begin(), hash->keys.end(), [&](shared_ptr<LiteralType> lit) {
                    return cast_type<ClassType>(lit->underlying.get())->symbol == core::Symbols::Symbol() &&
                           lit->value == spec.name._id;
                });
                if (arg == hash->keys.end()) {
                    if (!spec.isOptional()) {
                        missingArg(ctx, callLoc, fun, spec.ref(ctx));
                    }
                    continue;
                }
                consumed.insert(spec.name);
                TypeAndOrigins tpe;
                tpe.origins = args.back().origins;
                tpe.type = hash->values[arg - hash->keys.begin()];
                matchArgType(ctx, callLoc, this->symbol, fun, tpe, spec, fullType, targs);
            }
            for (auto &key : hash->keys) {
                SymbolRef klass = cast_type<ClassType>(key->underlying.get())->symbol;
                if (klass == core::Symbols::Symbol() &&
                    consumed.find(NameRef(ctx.state, key->value)) != consumed.end()) {
                    continue;
                }
                NameRef arg(ctx.state, key->value);

                if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Unrecognized keyword argument `{}` passed for method `{}`.", arg.toString(ctx),
                                fun.toString(ctx));
                }
            }
        } else if (hashArg.type->derivesFrom(ctx, core::Symbols::Hash())) {
            --aend;
            if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::MethodArgumentMismatch)) {
                e.setHeader("Passing an untyped hash to keyword arguments");
                e.addErrorSection(core::ErrorSection("Got " + hashArg.type->show(ctx) + " originating from:",
                                                     hashArg.origins2Explanations(ctx)));
            }
        }
    }
    if (hasKwargs && aend == args.end()) {
        // We have keyword arguments, but we didn't consume a hash at the
        // end. Report an error for each missing required keyword arugment.
        for (auto &spec : data.arguments()) {
            if (!spec.data(ctx).isKeyword() || spec.data(ctx).isOptional() || spec.data(ctx).isRepeated()) {
                continue;
            }
            missingArg(ctx, callLoc, fun, spec);
        }
    }

    if (ait != aend) {
        if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::MethodArgumentCountMismatch)) {
            e.setHeader("Too many arguments provided for method `{}`. Expected: `{}`, provided: `{}`",
                        fun.toString(ctx),

                        // TODO(nelhage): report actual counts of required arguments,
                        // and account for keyword arguments
                        data.arguments().size(), aend - args.begin());
        }
    }

    if (block != nullptr) {
        core::SymbolRef bspec;
        if (!data.arguments().empty()) {
            bspec = data.arguments().back();
        }
        if (!bspec.exists() || !bspec.data(ctx).isBlockArgument()) {
            // TODO(nelhage): passing a block to a function that does not accept one
        } else {
            *block = Types::resultTypeAsSeenFrom(ctx, bspec, this->symbol, targs);
        }
    }

    shared_ptr<Type> resultType = Types::resultTypeAsSeenFrom(ctx, method, this->symbol, targs);
    if (!resultType) {
        resultType = Types::dynamic();
    }
    return resultType;
}

shared_ptr<Type> ClassType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                                         shared_ptr<Type> fullType, shared_ptr<Type> *block) {
    vector<shared_ptr<Type>> empty;
    return dispatchCallWithTargs(ctx, fun, callLoc, args, fullType, empty, block);
}

shared_ptr<Type> ClassType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    if (isDynamic()) {
        return Types::dynamic();
    }
    core::SymbolRef method = this->symbol.data(ctx).findMemberTransitive(ctx, name);

    if (method.exists()) {
        const core::Symbol &data = method.data(ctx);

        if (data.arguments().size() > i) { // todo: this should become actual argument matching
            shared_ptr<Type> resultType = data.arguments()[i].data(ctx).resultType;
            if (!resultType) {
                resultType = Types::dynamic();
            }
            return resultType;
        } else {
            return Types::dynamic();
        }
    } else {
        return Types::dynamic();
    }
}

std::shared_ptr<Type> AliasType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                              std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                              shared_ptr<Type> fullType, shared_ptr<Type> *block) {
    Error::raise("AliasType::dispatchCall");
}

std::shared_ptr<Type> AliasType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    Error::raise("AliasType::getCallArgumentType");
}

std::shared_ptr<Type> MetaType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                             std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                             std::shared_ptr<Type> fullType, shared_ptr<Type> *block) {
    switch (name._id) {
        case core::Names::new_()._id: {
            wrapped->dispatchCall(ctx, core::Names::initialize(), callLoc, args, wrapped, wrapped, block);
            return wrapped;
        }
        default: {
            if (auto e = ctx.state.beginError(callLoc, core::errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
            }
            return Types::dynamic();
        }
    }
}

std::shared_ptr<Type> MetaType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    Error::raise("should never happen");
}
