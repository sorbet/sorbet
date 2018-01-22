#include "common/common.h"
#include "core/Names/core.h"
#include "core/Types.h"
#include "core/errors/infer.h"
#include <algorithm> // find_if, sort
#include <unordered_set>

template class std::vector<ruby_typer::core::SymbolRef>;
using namespace ruby_typer;
using namespace ruby_typer::core;
using namespace std;

shared_ptr<Type> ProxyType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> fullType, bool hasBlock) {
    categoryCounterInc("dispatch_call", "proxytype");
    return underlying->dispatchCall(ctx, name, callLoc, args, fullType, hasBlock);
}

shared_ptr<Type> ProxyType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return underlying->getCallArgumentType(ctx, name, i);
}

shared_ptr<Type> OrType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                      vector<TypeAndOrigins> &args, shared_ptr<Type> fullType, bool hasBlock) {
    categoryCounterInc("dispatch_call", "ortype");
    return Types::lub(ctx, left->dispatchCall(ctx, name, callLoc, args, fullType, hasBlock),
                      right->dispatchCall(ctx, name, callLoc, args, fullType, hasBlock));
}

shared_ptr<Type> OrType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return left->getCallArgumentType(ctx, name, i); // TODO: should glb with right
}

shared_ptr<Type> AndType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                       vector<TypeAndOrigins> &args, shared_ptr<Type> fullType, bool hasBlock) {
    categoryCounterInc("dispatch_call", "andtype");
    Error::notImplemented();
}

shared_ptr<Type> AndType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return Types::lub(ctx, left->getCallArgumentType(ctx, name, i), right->getCallArgumentType(ctx, name, i));
}

shared_ptr<Type> ShapeType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> fullType, bool hasBlock) {
    categoryCounterInc("dispatch_call", "shapetype");
    return ProxyType::dispatchCall(ctx, fun, callLoc, args, fullType, hasBlock);
}

shared_ptr<Type> MagicType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> fullType, bool hasBlock) {
    categoryCounterInc("dispatch_call", "magictype");
    switch (fun._id) {
        case Names::buildHash()._id: {
            ENFORCE(args.size() % 2 == 0);

            vector<shared_ptr<LiteralType>> keys;
            vector<shared_ptr<Type>> values;
            for (int i = 0; i < args.size(); i += 2) {
                auto *key = cast_type<LiteralType>(args[i].type.get());
                if (key == nullptr) {
                    return core::Types::hashClass();
                }

                // HACK(nelhage): clone the LiteralType by hand, since there's no way to go
                // from shared_ptr<Type> to shared_ptr<LiteralType>
                auto lit = make_unique<LiteralType>(key->value);
                lit->underlying = key->underlying;

                keys.push_back(move(lit));
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
            return ProxyType::dispatchCall(ctx, fun, callLoc, args, fullType, hasBlock);
    }
}

namespace {
void matchArgType(core::Context ctx, core::Loc callLoc, core::SymbolRef method, TypeAndOrigins &argTpe,
                  core::Symbol &argSym) {
    shared_ptr<Type> expectedType = argSym.resultType;
    if (!expectedType) {
        expectedType = Types::dynamic();
    }

    if (Types::isSubType(ctx, argTpe.type, expectedType)) {
        return;
    }
    ctx.state.error(core::ComplexError(
        callLoc, core::errors::Infer::MethodArgumentMismatch,
        "Argument " + argSym.name.toString(ctx) + " does not match expected type.",
        {core::ErrorSection(
             "Expected " + expectedType->toString(ctx),
             {
                 core::ErrorLine::from(argSym.definitionLoc, "Method {} has specified type of argument {} as {}",
                                       method.info(ctx).name.toString(ctx), argSym.name.toString(ctx),
                                       expectedType->toString(ctx)),
             }),
         core::ErrorSection("Got " + argTpe.type->toString(ctx) + " originating from:",
                            argTpe.origins2Explanations(ctx))}));
}

void missingArg(Context ctx, Loc callLoc, core::NameRef method, SymbolRef arg) {
    ctx.state.error(callLoc, core::errors::Infer::MethodArgumentCountMismatch,
                    "Missing required keyword argument {} for method {}.", arg.info(ctx).name.toString(ctx),
                    method.toString(ctx));
}
}; // namespace

// Guess overload. The way we guess is only arity based - we will return the overload that has the smallest number of
// arguments that is >= args.size()
core::SymbolRef guessOverload(core::Context ctx, core::SymbolRef primary, vector<TypeAndOrigins> &args,
                              shared_ptr<Type> fullType, bool hasBlock) {
    counterInc("calls.overloaded_invocations");
    Error::check(ctx.permitOverloadDefinitions(), "overload not permitted here");
    core::SymbolRef fallback = primary;
    vector<core::SymbolRef> allCandidates;

    allCandidates.push_back(primary);
    { // create candidates and sort them by number of arguments(stable by symbol id)
        int i = 0;
        core::SymbolRef current = primary;
        while (current.info(ctx).isOverloaded()) {
            i++;
            core::NameRef overloadName =
                ctx.state.freshNameUnique(core::UniqueNameKind::Overload, primary.info(ctx).name, i);
            core::SymbolRef overload = primary.info(ctx).owner.info(ctx).findMember(ctx, overloadName);
            if (!overload.exists()) {
                Error::raise("Corruption of overloads?");
            } else {
                allCandidates.push_back(overload);
                current = overload;
            }
        }

        sort(allCandidates.begin(), allCandidates.end(), [&](core::SymbolRef s1, core::SymbolRef s2) -> bool {
            if (s1.info(ctx).argumentsOrMixins.size() < s2.info(ctx).argumentsOrMixins.size())
                return true;
            if (s1.info(ctx).argumentsOrMixins.size() == s2.info(ctx).argumentsOrMixins.size()) {
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
                if (i >= candidate.info(ctx).argumentsOrMixins.size()) {
                    it = leftCandidates.erase(it);
                    continue;
                }

                auto argType = candidate.info(ctx).argumentsOrMixins[i].info(ctx).resultType;
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
            auto args = candidate.info(ctx).argumentsOrMixins;
            if (args.size() == 0) {
                if (hasBlock) {
                    it = leftCandidates.erase(it);
                    continue;
                }
            } else {
                auto &lastArg = args.back().info(ctx);
                if (lastArg.isBlockArgument() != hasBlock) {
                    it = leftCandidates.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

    { // keep only candidates with closes arity
        struct Comp {
            core::Context &ctx;

            bool operator()(core::SymbolRef s, int i) const {
                return s.info(ctx).argumentsOrMixins.size() < i;
            }
            bool operator()(int i, core::SymbolRef s) const {
                return i < s.info(ctx).argumentsOrMixins.size();
            }
            Comp(core::Context &ctx) : ctx(ctx){};
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

// This implements Ruby's argument matching logic (assigning values passed to a
// method call to formal parameters of the method).
//
// Known incompleteness or inconsistencies with Ruby:
//  - Missing coercion to keyword arguments via `#to_hash`
//  - Missing handling of block arguments
//  - We never allow a non-shaped Hash to satisfy keyword arguments;
//    We should, at a minimum, probably allow one to satisfy an **kwargs : dynamic
//    (with a subtype check on the key type, once we have generics)
shared_ptr<Type> ClassType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> fullType, bool hasBlock) {
    categoryCounterInc("dispatch_call", "classtype");
    if (isDynamic()) {
        return Types::dynamic();
    }
    core::SymbolRef mayBeOverloaded = this->symbol.info(ctx).findMemberTransitive(ctx, fun);

    if (!mayBeOverloaded.exists()) {
        string maybeComponent;
        if (fullType.get() != this) {
            maybeComponent = " component of " + fullType->toString(ctx);
        }
        ctx.state.error(callLoc, core::errors::Infer::UnknownMethod, "Method {} does not exist on {}{}",
                        fun.name(ctx).toString(ctx), this->toString(ctx), maybeComponent);
        return Types::dynamic();
    }

    core::SymbolRef method =
        mayBeOverloaded.info(ctx).isOverloaded()
            ? guessOverload(ctx.withOwner(mayBeOverloaded), mayBeOverloaded, args, fullType, hasBlock)
            : mayBeOverloaded;

    core::Symbol &info = method.info(ctx);

    bool hasKwargs = std::any_of(info.arguments().begin(), info.arguments().end(),
                                 [&ctx](core::SymbolRef arg) { return arg.info(ctx).isKeyword(); });

    auto pit = info.arguments().begin();
    auto pend = info.arguments().end();

    if (pit != pend && (pend - 1)->info(ctx).isBlockArgument()) {
        --pend;
    }

    auto ait = args.begin();
    auto aend = args.end();

    while (pit != pend && ait != aend) {
        core::Symbol &spec = pit->info(ctx);
        auto &arg = *ait;
        if (spec.isKeyword()) {
            break;
        }
        if (ait + 1 == aend && hasKwargs && arg.type->derivesFrom(ctx, ctx.state.defn_Hash()) &&
            (spec.isOptional() || spec.isRepeated())) {
            break;
        }
        if (!spec.isRepeated()) {
            ++pit;
        }
        ++ait;

        matchArgType(ctx, callLoc, method, arg, spec);
    }

    if (pit != pend) {
        if (!(pit->info(ctx).isKeyword() || pit->info(ctx).isOptional() || pit->info(ctx).isRepeated() ||
              pit->info(ctx).isBlockArgument())) {
            ctx.state.error(callLoc, core::errors::Infer::MethodArgumentCountMismatch,
                            "Not enough arguments provided for method {}.\n Expected: {}, provided: {}",
                            fun.toString(ctx),

                            // TODO(nelhage): report actual counts of required arguments,
                            // and account for keyword arguments
                            info.arguments().size(),
                            args.size()); // TODO: should use position and print the source tree, not the cfg one.
        }
    }

    if (hasKwargs && ait != aend) {
        std::unordered_set<NameRef> consumed;
        auto &hashArg = *(aend - 1);

        // find keyword arguments and advance `pend` before them; We'll walk
        // `kwit` ahead below
        auto kwit = pit;
        while (!kwit->info(ctx).isKeyword()) {
            kwit++;
        }
        pend = kwit;

        if (hashArg.type->isDynamic()) {
            // Allow an untyped arg to satisfy all kwargs
            --aend;
        } else if (ShapeType *hash = cast_type<ShapeType>(hashArg.type.get())) {
            --aend;

            while (kwit != info.arguments().end()) {
                core::Symbol &spec = kwit->info(ctx);
                if (spec.isBlockArgument()) {
                    break;
                } else if (spec.isRepeated()) {
                    for (auto it = hash->keys.begin(); it != hash->keys.end(); ++it) {
                        auto key = *it;
                        SymbolRef klass = cast_type<ClassType>(key->underlying.get())->symbol;
                        if (klass != ctx.state.defn_Symbol()) {
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
                        matchArgType(ctx, callLoc, method, tpe, spec);
                    }
                    break;
                }
                ++kwit;

                auto arg = find_if(hash->keys.begin(), hash->keys.end(), [&](shared_ptr<LiteralType> lit) {
                    return cast_type<ClassType>(lit->underlying.get())->symbol == ctx.state.defn_Symbol() &&
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
                matchArgType(ctx, callLoc, method, tpe, spec);
            }
            for (auto &key : hash->keys) {
                SymbolRef klass = cast_type<ClassType>(key->underlying.get())->symbol;
                if (klass == ctx.state.defn_Symbol() &&
                    consumed.find(NameRef(ctx.state, key->value)) != consumed.end()) {
                    continue;
                }
                NameRef arg(ctx.state, key->value);

                ctx.state.error(callLoc, core::errors::Infer::MethodArgumentCountMismatch,
                                "Unrecognized keyword argument {} passed for method {}.", arg.toString(ctx),
                                fun.toString(ctx));
            }
        } else if (hashArg.type->derivesFrom(ctx, ctx.state.defn_Hash())) {
            --aend;
            ctx.state.error(core::ComplexError(
                callLoc, core::errors::Infer::MethodArgumentMismatch, "Passing an untyped hash to keyword arguments",
                {core::ErrorSection("Got " + hashArg.type->toString(ctx) + " originating from:",
                                    hashArg.origins2Explanations(ctx))}));
        }
    }
    if (hasKwargs && aend == args.end()) {
        // We have keyword arguments, but we didn't consume a hash at the
        // end. Report an error for each missing required keyword arugment.
        for (auto &spec : info.arguments()) {
            if (!spec.info(ctx).isKeyword() || spec.info(ctx).isOptional() || spec.info(ctx).isRepeated()) {
                continue;
            }
            missingArg(ctx, callLoc, fun, spec);
        }
    }

    if (ait != aend) {
        ctx.state.error(callLoc, core::errors::Infer::MethodArgumentCountMismatch,
                        "Too many arguments provided for method {}.\n Expected: {}, provided: {}", fun.toString(ctx),

                        // TODO(nelhage): report actual counts of required arguments,
                        // and account for keyword arguments
                        info.arguments().size(),
                        aend - args.begin()); // TODO: should use position and print the source tree, not the cfg one.
    }

    shared_ptr<Type> resultType = method.info(ctx).resultType;
    if (!resultType) {
        resultType = Types::dynamic();
    }
    return resultType;
}

shared_ptr<Type> ClassType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    if (isDynamic()) {
        return Types::dynamic();
    }
    core::SymbolRef method = this->symbol.info(ctx).findMemberTransitive(ctx, name);

    if (method.exists()) {
        core::Symbol &info = method.info(ctx);

        if (info.arguments().size() > i) { // todo: this should become actual argument matching
            shared_ptr<Type> resultType = info.arguments()[i].info(ctx).resultType;
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
                                              std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                              bool hasBlock) {
    Error::raise("AliasType::dispatchCall");
}

std::shared_ptr<Type> AliasType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    Error::raise("AliasType::getCallArgumentType");
}
