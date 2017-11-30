#include "common/common.h"
#include "core/Types.h"
#include <algorithm> // find_if
#include <unordered_set>

using namespace ruby_typer;
using namespace ruby_typer::core;
using namespace std;

shared_ptr<Type> ProxyType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> fullType) {
    return underlying->dispatchCall(ctx, name, callLoc, args, fullType);
}

shared_ptr<Type> ProxyType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return underlying->getCallArgumentType(ctx, name, i);
}

shared_ptr<Type> OrType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                      vector<TypeAndOrigins> &args, shared_ptr<Type> fullType) {
    return Types::lub(ctx, left->dispatchCall(ctx, name, callLoc, args, fullType),
                      right->dispatchCall(ctx, name, callLoc, args, fullType));
}

shared_ptr<Type> OrType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return left->getCallArgumentType(ctx, name, i); // TODO: should glb with right
}

shared_ptr<Type> AndType::dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                       vector<TypeAndOrigins> &args, shared_ptr<Type> fullType) {
    Error::notImplemented();
}

shared_ptr<Type> AndType::getCallArgumentType(core::Context ctx, core::NameRef name, int i) {
    return Types::lub(ctx, left->getCallArgumentType(ctx, name, i), right->getCallArgumentType(ctx, name, i));
}

shared_ptr<Type> HashType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                        vector<TypeAndOrigins> &args, shared_ptr<Type> fullType) {
    return ProxyType::dispatchCall(ctx, fun, callLoc, args, fullType);
}

shared_ptr<Type> MagicType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> fullType) {
    switch (fun._id) {
        case Names::buildHash()._id: {
            Error::check(args.size() % 2 == 0);

            vector<shared_ptr<LiteralType>> keys;
            vector<shared_ptr<Type>> values;
            for (int i = 0; i < args.size(); i += 2) {
                auto *key = dynamic_cast<LiteralType *>(args[i].type.get());
                if (key == nullptr) {
                    return make_unique<ClassType>(ctx.state.defn_Hash());
                }

                // HACK(nelhage): clone the LiteralType by hand, since there's no way to go
                // from shared_ptr<Type> to shared_ptr<LiteralType>
                auto lit = make_unique<LiteralType>(key->value);
                lit->underlying = key->underlying;

                keys.push_back(move(lit));
                values.push_back(args[i + 1].type);
            }
            return make_unique<HashType>(keys, values);
        }

        case Names::buildArray()._id: {
            vector<shared_ptr<Type>> elems;
            for (auto &elem : args) {
                elems.push_back(elem.type);
            }
            return make_unique<ArrayType>(elems);
        }
        default:
            return ProxyType::dispatchCall(ctx, fun, callLoc, args, fullType);
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
    ctx.state.errors.error(core::Reporter::ComplexError(
        callLoc, core::ErrorClass::MethodArgumentMismatch,
        "Argument " + argSym.name.toString(ctx) + " does not match expected type.",
        {core::Reporter::ErrorSection(
             "Expected " + expectedType->toString(ctx),
             {
                 core::Reporter::ErrorLine::from(
                     argSym.definitionLoc, "Method {} has specified type of argument {} as {}",
                     method.info(ctx).name.toString(ctx), argSym.name.toString(ctx), expectedType->toString(ctx)),
             }),
         core::Reporter::ErrorSection("Got " + argTpe.type->toString(ctx) + " originating from:",
                                      argTpe.origins2Explanations(ctx))}));
}

void missingArg(Context ctx, Loc callLoc, core::NameRef method, SymbolRef arg) {
    ctx.state.errors.error(callLoc, core::ErrorClass::MethodArgumentCountMismatch,
                           "Missing required keyword argument {} for method {}.", arg.info(ctx).name.toString(ctx),
                           method.toString(ctx));
}
}; // namespace

// This implements Ruby's argument matching logic (assigning values passed to a
// method call to formal parameters of the method).
//
// Known incompleteness or inconsistencies with Ruby:
//  - Missing handling of repeated keyword arguments (**kwargs)
//  - Missing coercion to keyword arguments via `#to_hash`
//  - Missing handling of block arguments
//  - We never allow a non-shaped Hash to satisfy keyword arguments;
//    We should, at a minimum, probably allow one to satisfy an **kwargs : dynamic
//    (with a subtype check on the key type, once we have generics)
shared_ptr<Type> ClassType::dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                         vector<TypeAndOrigins> &args, shared_ptr<Type> fullType) {
    if (isDynamic()) {
        return Types::dynamic();
    }
    core::SymbolRef method = this->symbol.info(ctx).findMemberTransitive(ctx, fun);

    if (!method.exists()) {
        string maybeComponent;
        if (fullType.get() != this) {
            maybeComponent = " component of " + fullType->toString(ctx);
        }
        ctx.state.errors.error(callLoc, core::ErrorClass::UnknownMethod, "Method {} does not exist on {}{}",
                               fun.name(ctx).toString(ctx), this->toString(ctx), maybeComponent);
        return Types::dynamic();
    }
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
            ctx.state.errors.error(
                callLoc, core::ErrorClass::MethodArgumentCountMismatch,
                "Not enough arguments provided for method {}.\n Expected: {}, provided: {}", fun.toString(ctx),

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
        } else if (HashType *hash = dynamic_cast<HashType *>(hashArg.type.get())) {
            --aend;

            while (kwit != info.arguments().end()) {
                core::Symbol &spec = kwit->info(ctx);
                if (spec.isRepeated() || spec.isBlockArgument()) {
                    break;
                }
                ++kwit;

                auto arg = find_if(hash->keys.begin(), hash->keys.end(), [&](shared_ptr<LiteralType> lit) {
                    return dynamic_cast<ClassType *>(lit->underlying.get())->symbol == ctx.state.defn_Symbol() &&
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
                SymbolRef klass = dynamic_cast<ClassType *>(key->underlying.get())->symbol;
                if (klass == ctx.state.defn_Symbol() && consumed.find(NameRef(key->value)) != consumed.end()) {
                    continue;
                }
                NameRef arg(key->value);

                ctx.state.errors.error(callLoc, core::ErrorClass::MethodArgumentCountMismatch,
                                       "Unrecognized keyword argument {} passed for method {}.", arg.toString(ctx),
                                       fun.toString(ctx));
            }
        } else if (hashArg.type->derivesFrom(ctx, ctx.state.defn_Hash())) {
            --aend;
            ctx.state.errors.error(core::Reporter::ComplexError(
                callLoc, core::ErrorClass::MethodArgumentMismatch, "Passing an untyped hash to keyword arguments",
                {core::Reporter::ErrorSection("Got " + hashArg.type->toString(ctx) + " originating from:",
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
        ctx.state.errors.error(
            callLoc, core::ErrorClass::MethodArgumentCountMismatch,
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
