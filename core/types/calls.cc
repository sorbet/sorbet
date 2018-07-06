#include "common/common.h"
#include "core/Names/core.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"
#include "core/errors/infer.h"
#include <algorithm> // find_if, sort
#include <unordered_set>

template class std::vector<sorbet::core::SymbolRef>;
using namespace std;

namespace sorbet {
namespace core {

DispatchResult ProxyType::dispatchCall(Context ctx, NameRef name, Loc callLoc, vector<TypeAndOrigins> &args,
                                       shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                       shared_ptr<SendAndBlockLink> block) {
    categoryCounterInc("dispatch_call", "proxytype");
    return underlying->dispatchCall(ctx, name, callLoc, args, underlying, fullType, block);
}

shared_ptr<Type> ProxyType::getCallArgumentType(Context ctx, NameRef name, int i) {
    return underlying->getCallArgumentType(ctx, name, i);
}

DispatchResult OrType::dispatchCall(Context ctx, NameRef name, Loc callLoc, vector<TypeAndOrigins> &args,
                                    shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                    shared_ptr<SendAndBlockLink> block) {
    categoryCounterInc("dispatch_call", "ortype");
    auto leftRet = left->dispatchCall(ctx, name, callLoc, args, left, fullType, block);
    auto rightRet = right->dispatchCall(ctx, name, callLoc, args, right, fullType, block);
    DispatchResult::ComponentVec components = move(leftRet.components);
    components.insert(components.end(), make_move_iterator(rightRet.components.begin()),
                      make_move_iterator(rightRet.components.end()));
    DispatchResult ret{
        Types::any(ctx, leftRet.returnType, rightRet.returnType),
        move(components),
    };
    return ret;
}

shared_ptr<Type> OrType::getCallArgumentType(Context ctx, NameRef name, int i) {
    return left->getCallArgumentType(ctx, name, i); // TODO: should glb with right
}

DispatchResult AppliedType::dispatchCall(Context ctx, NameRef name, Loc callLoc, vector<TypeAndOrigins> &args,
                                         shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                         shared_ptr<SendAndBlockLink> block) {
    categoryCounterInc("dispatch_call", "appliedType");
    ClassType ct(this->klass);
    return ct.dispatchCallWithTargs(ctx, name, callLoc, args, selfRef, fullType, this->targs, block);
}

DispatchResult TypeVar::dispatchCall(Context ctx, NameRef name, Loc callLoc, vector<TypeAndOrigins> &args,
                                     shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                     shared_ptr<SendAndBlockLink> block) {
    Error::raise("should never happen");
}

DispatchResult AndType::dispatchCall(Context ctx, NameRef name, Loc callLoc, vector<TypeAndOrigins> &args,
                                     shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                     shared_ptr<SendAndBlockLink> block) {
    categoryCounterInc("dispatch_call", "andtype");
    auto leftRet = left->dispatchCall(ctx, name, callLoc, args, left, fullType, block);
    auto rightRet = right->dispatchCall(ctx, name, callLoc, args, right, fullType, block);

    // If either side is missing the method, dispatch to the other.
    auto leftOk =
        all_of(leftRet.components.begin(), leftRet.components.end(), [&](auto &comp) { return comp.method.exists(); });
    auto rightOk = all_of(rightRet.components.begin(), rightRet.components.end(),
                          [&](auto &comp) { return comp.method.exists(); });
    if (leftOk && !rightOk) {
        return leftRet;
    }
    if (rightOk && !leftOk) {
        return rightRet;
    }

    DispatchResult::ComponentVec components = move(leftRet.components);
    components.insert(components.end(), make_move_iterator(rightRet.components.begin()),
                      make_move_iterator(rightRet.components.end()));
    DispatchResult ret{
        Types::all(ctx, leftRet.returnType, rightRet.returnType),
        move(components),
    };
    return ret;
}

shared_ptr<Type> AndType::getCallArgumentType(Context ctx, NameRef name, int i) {
    auto l = left->getCallArgumentType(ctx, name, i);
    auto r = right->getCallArgumentType(ctx, name, i);
    if (l == nullptr) {
        return r;
    }
    if (r == nullptr) {
        return l;
    }
    return Types::any(ctx, l, r);
}

DispatchResult ShapeType::dispatchCall(Context ctx, NameRef fun, Loc callLoc, vector<TypeAndOrigins> &args,
                                       shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                       shared_ptr<SendAndBlockLink> block) {
    categoryCounterInc("dispatch_call", "shapetype");
    return ProxyType::dispatchCall(ctx, fun, callLoc, args, selfRef, fullType, block);
}

DispatchResult TupleType::dispatchCall(Context ctx, NameRef fun, Loc callLoc, vector<TypeAndOrigins> &args,
                                       shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                       shared_ptr<SendAndBlockLink> block) {
    auto method = Symbols::Tuple().data(ctx).findMember(ctx, fun);
    if (method.exists() && method.data(ctx).intrinsic != nullptr) {
        auto result = method.data(ctx).intrinsic->apply(ctx, callLoc, args, selfRef, fullType, block);
        if (result != nullptr) {
            DispatchResult::ComponentVec components(1);
            components.front().receiver = selfRef;
            components.front().method = method;
            return DispatchResult{result, move(components)};
        }
    }
    return ProxyType::dispatchCall(ctx, fun, callLoc, args, selfRef, fullType, block);
}

namespace {
bool isSetter(Context ctx, NameRef fun) {
    if (fun.data(ctx).kind != NameKind::UTF8) {
        return false;
    }
    return fun.data(ctx).raw.utf8.back() == '=';
}

unique_ptr<BasicError> matchArgType(Context ctx, TypeConstraint &constr, Loc callLoc, SymbolRef inClass, NameRef fun,
                                    TypeAndOrigins &argTpe, const Symbol &argSym, shared_ptr<Type> &fullType,
                                    vector<shared_ptr<Type>> &targs, bool mayBeSetter = false) {
    shared_ptr<Type> expectedType = Types::resultTypeAsSeenFrom(ctx, argSym.ref(ctx), inClass, targs);
    if (!expectedType) {
        expectedType = Types::untyped();
    }

    expectedType = Types::replaceSelfType(
        ctx, expectedType,
        fullType); // TODO: fullType is actually not the best here. We want the last AND component of fullType

    if (Types::isSubTypeUnderConstraint(ctx, constr, argTpe.type, expectedType)) {
        return nullptr;
    }
    if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentMismatch)) {
        if (mayBeSetter && isSetter(ctx, fun)) {
            e.setHeader("Assigning a value to `{}` that does not match expected type `{}`", argSym.name.toString(ctx),
                        expectedType->show(ctx));
        } else {
            e.setHeader("`{}` doesn't match `{}` for argument `{}`", argTpe.type->show(ctx), expectedType->show(ctx),
                        argSym.name.toString(ctx));
            e.addErrorSection(ErrorSection({
                ErrorLine::from(argSym.definitionLoc, "Method `{}` has specified `{}` as `{}`",
                                argSym.owner.data(ctx).name.toString(ctx), argSym.name.toString(ctx),
                                expectedType->show(ctx)),
            }));
        }
        e.addErrorSection(
            ErrorSection("Got " + argTpe.type->show(ctx) + " originating from:", argTpe.origins2Explanations(ctx)));
        auto *ot = cast_type<OrType>(argTpe.type.get());
        if (ot) {
            auto *lt = cast_type<ClassType>(ot->left.get());
            auto *rt = cast_type<ClassType>(ot->right.get());
            if ((lt != nullptr && lt->symbol == Symbols::NilClass()) ||
                (rt != nullptr && rt->symbol == Symbols::NilClass())) {
                e.addErrorSection(ErrorSection("You could wrap it in `T.must()` before passing the argument."));
            }
        }
        return e.build();
    }
    return nullptr;
}

unique_ptr<BasicError> missingArg(Context ctx, Loc callLoc, NameRef method, SymbolRef arg) {
    if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentCountMismatch)) {
        e.setHeader("Missing required keyword argument `{}` for method `{}`", arg.data(ctx).name.toString(ctx),
                    method.toString(ctx));
        return e.build();
    }
    return nullptr;
}
}; // namespace

int getArity(Context ctx, SymbolRef method) {
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
SymbolRef guessOverload(Context ctx, SymbolRef inClass, SymbolRef primary, vector<TypeAndOrigins> &args,
                        shared_ptr<Type> fullType, vector<shared_ptr<Type>> &targs, bool hasBlock) {
    counterInc("calls.overloaded_invocations");
    ENFORCE(ctx.permitOverloadDefinitions(), "overload not permitted here");
    SymbolRef fallback = primary;
    vector<SymbolRef> allCandidates;

    allCandidates.push_back(primary);
    { // create candidates and sort them by number of arguments(stable by symbol id)
        int i = 0;
        SymbolRef current = primary;
        while (current.data(ctx).isOverloaded()) {
            i++;
            NameRef overloadName = ctx.state.getNameUnique(UniqueNameKind::Overload, primary.data(ctx).name, i);
            SymbolRef overload = primary.data(ctx).owner.data(ctx).findMember(ctx, overloadName);
            if (!overload.exists()) {
                Error::raise("Corruption of overloads?");
            } else {
                allCandidates.push_back(overload);
                current = overload;
            }
        }

        sort(allCandidates.begin(), allCandidates.end(), [&](SymbolRef s1, SymbolRef s2) -> bool {
            if (getArity(ctx, s1) < getArity(ctx, s2)) {
                return true;
            }
            if (getArity(ctx, s1) == getArity(ctx, s2)) {
                return s1._id < s2._id;
            }
            return false;
        });
    }

    vector<SymbolRef> leftCandidates = allCandidates;

    {
        // Lets see if we can filter them out using arguments.
        int i = -1;
        for (auto &arg : args) {
            i++;
            for (auto it = leftCandidates.begin(); it != leftCandidates.end(); /* nothing*/) {
                SymbolRef candidate = *it;
                if (i >= getArity(ctx, candidate)) {
                    it = leftCandidates.erase(it);
                    continue;
                }

                auto argType = Types::resultTypeAsSeenFrom(ctx, candidate.data(ctx).arguments()[i], inClass, targs);
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
            SymbolRef candidate = *it;
            auto args = candidate.data(ctx).arguments();
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
            Context ctx;

            bool operator()(SymbolRef s, int i) const {
                return getArity(ctx, s) < i;
            }

            bool operator()(int i, SymbolRef s) const {
                return i < getArity(ctx, s);
            }

            Comp(Context ctx) : ctx(ctx){};
        } cmp(ctx);

        auto er = equal_range(leftCandidates.begin(), leftCandidates.end(), args.size(), cmp);
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
    if (auto *metaType = cast_type<MetaType>(tp.get())) {
        return metaType->wrapped;
    }
    if (auto *classType = cast_type<ClassType>(tp.get())) {
        SymbolRef attachedClass = classType->symbol.data(ctx).attachedClass(ctx);
        if (!attachedClass.exists()) {
            if (auto e = ctx.state.beginError(loc, errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
            }
            return Types::untyped();
        }

        return attachedClass.data(ctx).externalType(ctx);
    }

    if (auto *shapeType = cast_type<ShapeType>(tp.get())) {
        vector<shared_ptr<Type>> unwrappedValues;
        for (auto value : shapeType->values) {
            unwrappedValues.emplace_back(unwrapType(ctx, loc, value));
        }
        return make_shared<ShapeType>(shapeType->keys, unwrappedValues);
    } else if (auto *tupleType = cast_type<TupleType>(tp.get())) {
        vector<shared_ptr<Type>> unwrappedElems;
        for (auto elem : tupleType->elems) {
            unwrappedElems.emplace_back(unwrapType(ctx, loc, elem));
        }
        return TupleType::build(ctx, unwrappedElems);
    } else if (auto *litType = cast_type<LiteralType>(tp.get())) {
        if (auto e = ctx.state.beginError(loc, errors::Infer::BareTypeUsage)) {
            e.setHeader("Unsupported usage of literal type");
        }
        return Types::untyped();
    }
    return tp;
}

// This implements Ruby's argument matching logic (assigning values passed to a
// method call to formal parameters of the method).
//
// Known incompleteness or inconsistencies with Ruby:
//  - Missing coercion to keyword arguments via `#to_hash`
//  - We never allow a non-shaped Hash to satisfy keyword arguments;
//    We should, at a minimum, probably allow one to satisfy an **kwargs : untyped
//    (with a subtype check on the key type, once we have generics)
DispatchResult ClassType::dispatchCallWithTargs(Context ctx, NameRef fun, Loc callLoc, vector<TypeAndOrigins> &args,
                                                shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                                vector<shared_ptr<Type>> &targs, shared_ptr<SendAndBlockLink> block) {
    categoryCounterInc("dispatch_call", "classtype");
    if (isUntyped()) {
        return DispatchResult(Types::untyped(), move(selfRef), Symbols::untyped());
    } else if (this->symbol == Symbols::void_()) {
        if (auto e = ctx.state.beginError(callLoc, errors::Infer::UnknownMethod)) {
            e.setHeader("Can not call method `{}` on void type", fun.data(ctx).toString(ctx));
        }
        return DispatchResult(Types::untyped(), move(selfRef), Symbols::noSymbol());
    }

    SymbolRef mayBeOverloaded = this->symbol.data(ctx).findMemberTransitive(ctx, fun);

    if (!mayBeOverloaded.exists()) {
        if (fun == Names::initialize()) {
            // Special-case initialize(). We should define this on
            // `BasicObject`, but our method-resolution order is wrong, and
            // putting it there will inadvertently shadow real definitions in
            // some cases, so we special-case it here as a last resort.
            auto result = DispatchResult(Types::untyped(), move(selfRef), Symbols::noSymbol());
            if (!args.empty()) {
                if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Wrong number of arguments for constructor. Expected: `{}`, got: `{}`", 0, args.size());
                    result.components.front().errors.emplace_back(e.build());
                }
            }
            return result;
        }
        auto result = DispatchResult(Types::untyped(), move(selfRef), Symbols::noSymbol());
        if (auto e = ctx.state.beginError(callLoc, errors::Infer::UnknownMethod)) {
            if (fullType.get() != this) {
                e.setHeader("Method `{}` does not exist on `{}` component of `{}`", fun.data(ctx).toString(ctx),
                            this->show(ctx), fullType->show(ctx));
                if (this->symbol == Symbols::NilClass()) {
                    e.addErrorSection(ErrorSection("You could wrap it in `T.must()` before calling the function."));
                }
            } else {
                e.setHeader("Method `{}` does not exist on `{}`", fun.data(ctx).toString(ctx), this->show(ctx));
                auto alternatives = this->symbol.data(ctx).findMemberFuzzyMatch(ctx, fun);
                if (!alternatives.empty()) {
                    vector<ErrorLine> lines;
                    for (auto alternative : alternatives) {
                        lines.emplace_back(ErrorLine::from(alternative.symbol.data(ctx).definitionLoc,
                                                           "Did you mean: `{}`?", alternative.name.toString(ctx)));
                    }
                    e.addErrorSection(ErrorSection(lines));
                }

                auto attached = this->symbol.data(ctx).attachedClass(ctx);
                if (attached.exists() && this->symbol.data(ctx).derivesFrom(ctx, Symbols::Chalk_Tools_Accessible())) {
                    e.addErrorSection(ErrorSection(
                        "If this method is generated by Chalk::Tools::Accessible, you "
                        "may need to re-generate the .rbi. Try running:\n" +
                        ErrorColors::format("  scripts/bin/remote-script sorbet/shim_generation/make_accessible.rb {}",
                                            attached.data(ctx).fullName(ctx))));
                }
            }
            result.components.front().errors.emplace_back(e.build());
        }
        return result;
    }

    SymbolRef method = mayBeOverloaded.data(ctx).isOverloaded()
                           ? guessOverload(ctx.withOwner(mayBeOverloaded), this->symbol, mayBeOverloaded, args,
                                           fullType, targs, block != nullptr)
                           : mayBeOverloaded;

    DispatchResult result;
    result.components.emplace_back(DispatchComponent{selfRef, method, vector<unique_ptr<BasicError>>()});

    const Symbol &data = method.data(ctx);
    unique_ptr<TypeConstraint> maybeConstraint;
    TypeConstraint *constr;
    if (block) {
        constr = block->constr.get();
    } else if (data.isGenericMethod()) {
        maybeConstraint = make_unique<TypeConstraint>();
        constr = maybeConstraint.get();
    } else {
        constr = &TypeConstraint::EmptyFrozenConstraint;
    }

    if (data.isGenericMethod()) {
        constr->defineDomain(ctx, data.typeArguments());
    }
    bool hasKwargs = any_of(data.arguments().begin(), data.arguments().end(),
                            [&ctx](SymbolRef arg) { return arg.data(ctx).isKeyword(); });

    auto pit = data.arguments().begin();
    auto pend = data.arguments().end();

    if (pit != pend && (pend - 1)->data(ctx).isBlockArgument()) {
        --pend;
    }

    auto ait = args.begin();
    auto aend = args.end();

    while (pit != pend && ait != aend) {
        const Symbol &spec = pit->data(ctx);
        auto &arg = *ait;
        if (spec.isKeyword()) {
            break;
        }
        if (ait + 1 == aend && hasKwargs && arg.type->derivesFrom(ctx, Symbols::Hash()) &&
            (spec.isOptional() || spec.isRepeated())) {
            break;
        }

        if (auto e =
                matchArgType(ctx, *constr, callLoc, this->symbol, fun, arg, spec, fullType, targs, args.size() == 1)) {
            result.components.front().errors.emplace_back(move(e));
        }

        if (!spec.isRepeated()) {
            ++pit;
        }
        ++ait;
    }

    if (pit != pend) {
        if (!(pit->data(ctx).isKeyword() || pit->data(ctx).isOptional() || pit->data(ctx).isRepeated() ||
              pit->data(ctx).isBlockArgument())) {
            if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentCountMismatch)) {
                e.setHeader("Not enough arguments provided for method `{}`. Expected: `{}`, got: `{}`",
                            fun.toString(ctx),
                            // TODO(nelhage): report actual counts of required arguments,
                            // and account for keyword arguments
                            data.arguments().size(),
                            args.size()); // TODO: should use position and print the source tree, not the cfg one.
                e.addErrorLine(method.data(ctx).definitionLoc, "`{}` defined here", fun.toString(ctx));
                result.components.front().errors.emplace_back(e.build());
            }
        }
    }

    if (hasKwargs && ait != aend) {
        unordered_set<NameRef> consumed;
        auto &hashArg = *(aend - 1);

        // find keyword arguments and advance `pend` before them; We'll walk
        // `kwit` ahead below
        auto kwit = pit;
        while (!kwit->data(ctx).isKeyword()) {
            kwit++;
        }
        pend = kwit;

        if (hashArg.type->isUntyped()) {
            // Allow an untyped arg to satisfy all kwargs
            --aend;
        } else if (auto *hash = cast_type<ShapeType>(hashArg.type.get())) {
            --aend;

            while (kwit != data.arguments().end()) {
                const Symbol &spec = kwit->data(ctx);
                if (spec.isBlockArgument()) {
                    break;
                } else if (spec.isRepeated()) {
                    for (auto it = hash->keys.begin(); it != hash->keys.end(); ++it) {
                        auto key = *it;
                        SymbolRef klass = cast_type<ClassType>(key->underlying.get())->symbol;
                        if (klass != Symbols::Symbol()) {
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
                        if (auto e =
                                matchArgType(ctx, *constr, callLoc, this->symbol, fun, tpe, spec, fullType, targs)) {
                            result.components.front().errors.emplace_back(move(e));
                        }
                    }
                    break;
                }
                ++kwit;

                auto arg = find_if(hash->keys.begin(), hash->keys.end(), [&](shared_ptr<LiteralType> lit) {
                    return cast_type<ClassType>(lit->underlying.get())->symbol == Symbols::Symbol() &&
                           lit->value == spec.name._id;
                });
                if (arg == hash->keys.end()) {
                    if (!spec.isOptional()) {
                        if (auto e = missingArg(ctx, callLoc, fun, spec.ref(ctx))) {
                            result.components.front().errors.emplace_back(move(e));
                        }
                    }
                    continue;
                }
                consumed.insert(spec.name);
                TypeAndOrigins tpe;
                tpe.origins = args.back().origins;
                tpe.type = hash->values[arg - hash->keys.begin()];
                if (auto e = matchArgType(ctx, *constr, callLoc, this->symbol, fun, tpe, spec, fullType, targs)) {
                    result.components.front().errors.emplace_back(move(e));
                }
            }
            for (auto &key : hash->keys) {
                SymbolRef klass = cast_type<ClassType>(key->underlying.get())->symbol;
                if (klass == Symbols::Symbol() && consumed.find(NameRef(ctx.state, key->value)) != consumed.end()) {
                    continue;
                }
                NameRef arg(ctx.state, key->value);

                if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Unrecognized keyword argument `{}` passed for method `{}`", arg.toString(ctx),
                                fun.toString(ctx));
                    result.components.front().errors.emplace_back(e.build());
                }
            }
        } else if (hashArg.type->derivesFrom(ctx, Symbols::Hash())) {
            --aend;
            if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentMismatch)) {
                e.setHeader("Passing an untyped hash to keyword arguments");
                e.addErrorSection(ErrorSection("Got " + hashArg.type->show(ctx) + " originating from:",
                                               hashArg.origins2Explanations(ctx)));
                result.components.front().errors.emplace_back(e.build());
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
            if (auto e = missingArg(ctx, callLoc, fun, spec)) {
                result.components.front().errors.emplace_back(move(e));
            }
        }
    }

    if (ait != aend) {
        if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentCountMismatch)) {
            e.setHeader("Too many arguments provided for method `{}`. Expected: `{}`, got: `{}`", fun.toString(ctx),

                        // TODO(nelhage): report actual counts of required arguments,
                        // and account for keyword arguments
                        data.arguments().size(), aend - args.begin());
            e.addErrorLine(method.data(ctx).definitionLoc, "`{}` defined here", fun.toString(ctx));
            result.components.front().errors.emplace_back(e.build());
        }
    }

    shared_ptr<Type> resultType = nullptr;

    if (method.data(ctx).intrinsic != nullptr) {
        resultType = method.data(ctx).intrinsic->apply(ctx, callLoc, args, selfRef, fullType, block);
    }

    if (resultType == nullptr) {
        resultType = Types::resultTypeAsSeenFrom(ctx, method, this->symbol, targs);
    }
    if (block == nullptr) {
        // if block is there we do not attempt to solve the constaint. CFG adds an explicit solve
        // node that triggers constraint solving
        if (!constr->solve(ctx)) {
            if (auto e = ctx.state.beginError(callLoc, errors::Infer::GenericMethodConstaintUnsolved)) {
                e.setHeader("Could not find valid instantiation of type parameters");
                result.components.front().errors.emplace_back(e.build());
            }
        }
    }

    if (!resultType) {
        resultType = Types::untyped();
    } else if (!constr->isEmpty() && constr->isSolved()) {
        resultType = Types::instantiate(ctx, resultType, *constr);
    }
    resultType = Types::replaceSelfType(
        ctx, resultType,
        fullType); // TODO: full type is not the best here. We actually want the last AND component of fulltype

    if (block != nullptr) {
        SymbolRef bspec;
        if (!data.arguments().empty()) {
            bspec = data.arguments().back();
        }
        if (!bspec.exists() || !bspec.data(ctx).isBlockArgument()) {
            // TODO(nelhage): passing a block to a function that does not accept one
        } else {
            shared_ptr<Type> blockType = Types::resultTypeAsSeenFrom(ctx, bspec, this->symbol, targs);
            if (!blockType) {
                blockType = Types::untyped();
            }

            block->returnTp = Types::getProcReturnType(ctx, blockType);
            blockType = constr->isSolved() ? Types::instantiate(ctx, blockType, *constr)
                                           : Types::approximate(ctx, blockType, *constr);

            block->blockPreType = blockType;
            block->sendTp = resultType;
        }
    }
    result.returnType = move(resultType);
    return result;
}

DispatchResult ClassType::dispatchCall(Context ctx, NameRef fun, Loc callLoc, vector<TypeAndOrigins> &args,
                                       shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                       shared_ptr<SendAndBlockLink> block) {
    vector<shared_ptr<Type>> empty;
    return dispatchCallWithTargs(ctx, fun, callLoc, args, selfRef, fullType, empty, block);
}

shared_ptr<Type> ClassType::getCallArgumentType(Context ctx, NameRef name, int i) {
    if (isUntyped()) {
        return Types::untyped();
    }
    SymbolRef method = this->symbol.data(ctx).findMemberTransitive(ctx, name);

    if (!method.exists()) {
        return nullptr;
    }
    const Symbol &data = method.data(ctx);

    if (i >= data.arguments().size()) { // todo: this should become actual argument matching
        return nullptr;
    }

    shared_ptr<Type> resultType = data.arguments()[i].data(ctx).resultType;
    if (!resultType) {
        resultType = Types::untyped();
    }
    return resultType;
}

shared_ptr<Type> AppliedType::getCallArgumentType(Context ctx, NameRef name, int i) {
    SymbolRef method = this->klass.data(ctx).findMemberTransitive(ctx, name);

    if (!method.exists()) {
        return nullptr;
    }

    const Symbol &data = method.data(ctx);

    if (i >= data.arguments().size()) { // todo: this should become actual argument matching
        return nullptr;
    }

    shared_ptr<Type> resultType = Types::resultTypeAsSeenFrom(ctx, data.arguments()[i], this->klass, this->targs);
    if (!resultType) {
        resultType = Types::untyped();
    }
    return resultType;
}

DispatchResult AliasType::dispatchCall(Context ctx, NameRef name, Loc callLoc, vector<TypeAndOrigins> &args,
                                       shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                       shared_ptr<SendAndBlockLink> block) {
    Error::raise("AliasType::dispatchCall");
}

shared_ptr<Type> AliasType::getCallArgumentType(Context ctx, NameRef name, int i) {
    Error::raise("AliasType::getCallArgumentType");
}

DispatchResult MetaType::dispatchCall(Context ctx, NameRef name, Loc callLoc, vector<TypeAndOrigins> &args,
                                      shared_ptr<Type> selfRef, shared_ptr<Type> fullType,
                                      shared_ptr<SendAndBlockLink> block) {
    switch (name._id) {
        case Names::new_()._id: {
            auto res = DispatchResult(wrapped, selfRef, Symbols::Class().data(ctx).findMember(ctx, Names::new_()));
            auto inner = wrapped->dispatchCall(ctx, Names::initialize(), callLoc, args, wrapped, wrapped, block);
            res.components.insert(res.components.end(), make_move_iterator(inner.components.begin()),
                                  make_move_iterator(inner.components.end()));
            return res;
        }
        default: {
            auto res = DispatchResult(Types::untyped(), selfRef, Symbols::noSymbol());
            if (auto e = ctx.state.beginError(callLoc, errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
                res.components.front().errors.emplace_back(e.build());
            }
            return res;
        }
    }
}

shared_ptr<Type> MetaType::getCallArgumentType(Context ctx, NameRef name, int i) {
    Error::raise("should never happen");
}

SymbolRef unwrapSymbol(shared_ptr<Type> type) {
    SymbolRef result;
    while (!result.exists()) {
        typecase(type.get(),

                 [&](ClassType *klass) { result = klass->symbol; },

                 [&](AppliedType *app) { result = app->klass; },

                 [&](ProxyType *proxy) { type = proxy->underlying; },

                 [&](Type *ty) { ENFORCE(false, "Unexpected type: ", ty->typeName()); });
    }
    return result;
}
namespace {

class T_untyped : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        return Types::untyped();
    }
} T_untyped;

class T_must : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        if (args.empty()) {
            return nullptr;
        }
        if (!args[0].type->isFullyDefined()) {
            if (auto e = ctx.state.beginError(callLoc, errors::Infer::BareTypeUsage)) {
                e.setHeader("T.must() applied to incomplete type `{}`", args[0].type->show(ctx));
            }
            return nullptr;
        }
        auto ret = Types::approximateSubtract(ctx, args[0].type, Types::nilClass());
        if (ret == args[0].type) {
            if (auto e = ctx.state.beginError(callLoc, errors::Infer::InvalidCast)) {
                e.setHeader("T.must(): Expected a `T.nilable` type, got: `{}`", args[0].type->show(ctx));
            }
        }
        return ret;
    }
} T_must;

class T_any : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        if (args.empty()) {
            return Types::untyped();
        }

        shared_ptr<Type> res = Types::bottom();
        for (auto &arg : args) {
            auto ty = unwrapType(ctx, arg.origins[0], arg.type);
            res = Types::any(ctx, res, ty);
        }

        return make_shared<MetaType>(res);
    }
} T_any;

class T_all : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        if (args.empty()) {
            return Types::untyped();
        }

        shared_ptr<Type> res = Types::top();
        for (auto &arg : args) {
            auto ty = unwrapType(ctx, arg.origins[0], arg.type);
            res = Types::all(ctx, res, ty);
        }

        return make_shared<MetaType>(res);
    }
} T_all;

class T_revealType : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        if (args.empty()) {
            return Types::untyped();
        }

        if (auto e = ctx.state.beginError(callLoc, errors::Infer::RevealType)) {
            e.setHeader("Revealed type: `{}`", args[0].type->show(ctx));
            e.addErrorSection(ErrorSection("From:", args[0].origins2Explanations(ctx)));
        }
        return args[0].type;
    }
} T_revealType;

class Object_class : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        SymbolRef self = unwrapSymbol(selfRef);
        auto singleton = self.data(ctx).lookupSingletonClass(ctx);
        if (singleton.exists()) {
            return make_shared<ClassType>(singleton);
        }
        return Types::classClass();
    }
} Object_class;

class Class_new : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        SymbolRef self = unwrapSymbol(selfRef);

        auto attachedClass = self.data(ctx).attachedClass(ctx);
        if (!attachedClass.exists()) {
            if (self == Symbols::Class()) {
                // `Class.new(...)`, but it isn't a specific Class. We know
                // calling .new on a Class will yield some sort of Object
                attachedClass = Symbols::Object();
            } else {
                return nullptr;
            }
        }
        auto instanceTy = attachedClass.data(ctx).externalType(ctx);
        auto dispatched =
            instanceTy->dispatchCall(ctx, Names::initialize(), callLoc, args, instanceTy, instanceTy, linkType);
        // TODO(nelhage): Arguably apply() should also return a DispatchResult
        // and we should pass these errors up to the caller.
        for (auto &comp : dispatched.components) {
            for (auto &err : comp.errors) {
                ctx.state._error(move(err));
            }
        }
        return instanceTy;
    }
} Class_new;

class T_Generic_squareBrackets : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        SymbolRef attachedClass;

        SymbolRef self = unwrapSymbol(selfRef);
        attachedClass = self.data(ctx).attachedClass(ctx);

        if (!attachedClass.exists()) {
            return nullptr;
        }

        if (attachedClass == Symbols::T_Array()) {
            attachedClass = Symbols::Array();
        } else if (attachedClass == Symbols::T_Hash()) {
            attachedClass = Symbols::Hash();
        } else if (attachedClass == Symbols::T_Enumerable()) {
            attachedClass = Symbols::Enumerable();
        } else if (attachedClass == Symbols::T_Range()) {
            attachedClass = Symbols::Range();
        } else if (attachedClass == Symbols::T_Set()) {
            attachedClass = Symbols::Set();
        }

        auto arity = attachedClass.data(ctx).typeArity(ctx);
        if (attachedClass == Symbols::Hash()) {
            arity = 2;
        }
        if (attachedClass.data(ctx).typeMembers().empty()) {
            return nullptr;
        }

        if (args.size() != arity) {
            if (auto e = ctx.state.beginError(callLoc, errors::Infer::GenericArgumentCountMismatch)) {
                e.setHeader("Wrong number of type parameters for `{}`. Expected: `{}`, got: `{}`",
                            attachedClass.data(ctx).show(ctx), arity, args.size());
            }
        }

        vector<shared_ptr<Type>> targs;
        auto it = args.begin();
        int i = -1;
        for (auto mem : attachedClass.data(ctx).typeMembers()) {
            ++i;
            if (mem.data(ctx).isFixed()) {
                targs.emplace_back(mem.data(ctx).resultType);
            } else if (it != args.end()) {
                targs.emplace_back(unwrapType(ctx, it->origins[0], it->type));
                ++it;
            } else if (attachedClass == Symbols::Hash() && i == 2) {
                auto tupleArgs = targs;
                targs.emplace_back(TupleType::build(ctx, tupleArgs));
            } else {
                targs.emplace_back(Types::untyped());
            }
        }

        return make_shared<MetaType>(make_shared<AppliedType>(attachedClass, targs));
    }
} T_Generic_squareBrackets;

class Magic_buildHash : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        ENFORCE(args.size() % 2 == 0);

        vector<shared_ptr<LiteralType>> keys;
        vector<shared_ptr<Type>> values;
        for (int i = 0; i < args.size(); i += 2) {
            auto *key = cast_type<LiteralType>(args[i].type.get());
            if (key == nullptr) {
                return Types::hashOfUntyped();
            }

            keys.push_back(shared_ptr<LiteralType>(args[i].type, key));
            values.push_back(args[i + 1].type);
        }
        return make_unique<ShapeType>(keys, values);
    }
} Magic_buildHash;

class Magic_buildArray : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        if (args.empty()) {
            return Types::arrayOfUntyped();
        }
        vector<shared_ptr<Type>> elems;
        for (auto &elem : args) {
            elems.push_back(elem.type);
        }
        return TupleType::build(ctx, elems);
    }
} Magic_buildArray;

class Magic_expandSplat : public Symbol::IntrinsicMethod {
    static shared_ptr<Type> expandArray(Context ctx, shared_ptr<Type> type, int expandTo) {
        if (auto *ot = cast_type<OrType>(type.get())) {
            return Types::any(ctx, expandArray(ctx, ot->left, expandTo), expandArray(ctx, ot->right, expandTo));
        }

        auto *tuple = cast_type<TupleType>(type.get());
        if (tuple == nullptr && type->derivesFrom(ctx, Symbols::Array())) {
            // If this is an array and not a tuple, just pass it through. We
            // can't say anything about the elements.
            return type;
        }
        vector<shared_ptr<Type>> types;
        if (tuple) {
            types.insert(types.end(), tuple->elems.begin(), tuple->elems.end());
        } else {
            types.push_back(type);
        }
        if (types.size() < expandTo) {
            types.resize(expandTo, Types::nilClass());
        }

        return TupleType::build(ctx, types);
    }

public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        if (args.size() != 3) {
            return Types::arrayOfUntyped();
        }
        auto val = args.front().type;
        auto *beforeLit = cast_type<LiteralType>(args[1].type.get());
        auto *afterLit = cast_type<LiteralType>(args[2].type.get());
        if (!(beforeLit->underlying->derivesFrom(ctx, Symbols::Integer()) &&
              afterLit->underlying->derivesFrom(ctx, Symbols::Integer()))) {
            return Types::untyped();
        }
        int before = (int)beforeLit->value;
        int after = (int)afterLit->value;
        return expandArray(ctx, val, before + after);
    }
} Magic_expandSplat;

class Tuple_squareBrackets : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        auto *tuple = cast_type<TupleType>(selfRef.get());
        ENFORCE(tuple);
        LiteralType *lit = nullptr;
        if (args.size() == 1) {
            lit = cast_type<LiteralType>(args.front().type.get());
        }
        if (!lit || !lit->underlying->derivesFrom(ctx, Symbols::Integer())) {
            return nullptr;
        }

        auto idx = lit->value;
        if (idx < 0) {
            idx = tuple->elems.size() + idx;
        }
        if (idx >= tuple->elems.size()) {
            return Types::nilClass();
        }
        return tuple->elems[idx];
    }
} Tuple_squareBrackets;

class Tuple_last : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        auto *tuple = cast_type<TupleType>(selfRef.get());
        ENFORCE(tuple);

        if (!args.empty()) {
            return nullptr;
        }
        if (tuple->elems.empty()) {
            return Types::nilClass();
        }
        return tuple->elems.back();
    }
} Tuple_last;

class Tuple_first : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        auto *tuple = cast_type<TupleType>(selfRef.get());
        ENFORCE(tuple);

        if (!args.empty()) {
            return nullptr;
        }
        if (tuple->elems.empty()) {
            return Types::nilClass();
        }
        return tuple->elems.front();
    }
} Tuple_first;

class Tuple_minMax : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        auto *tuple = cast_type<TupleType>(selfRef.get());
        ENFORCE(tuple);

        if (!args.empty()) {
            return nullptr;
        }
        if (tuple->elems.empty()) {
            return Types::nilClass();
        }
        return tuple->elementType();
    }
} Tuple_minMax;

class Array_flatten : public Symbol::IntrinsicMethod {
    static shared_ptr<Type> recursivelyFlattenArrays(Context ctx, shared_ptr<Type> type) {
        shared_ptr<Type> result;

        typecase(type.get(),

                 [&](OrType *o) {
                     result = Types::any(ctx, recursivelyFlattenArrays(ctx, o->left),
                                         recursivelyFlattenArrays(ctx, o->right));
                 },

                 [&](AppliedType *a) {
                     if (a->klass != Symbols::Array()) {
                         result = type;
                         return;
                     }
                     ENFORCE(a->targs.size() == 1);
                     result = recursivelyFlattenArrays(ctx, a->targs.front());
                 },

                 [&](TupleType *t) { result = recursivelyFlattenArrays(ctx, t->elementType()); },

                 [&](Type *t) { result = move(type); });
        return result;
    }

public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        shared_ptr<Type> element;
        if (auto *ap = cast_type<AppliedType>(selfRef.get())) {
            ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(ctx).derivesFrom(ctx, Symbols::Array()));
            ENFORCE(!ap->targs.empty());
            element = ap->targs.front();
        } else if (auto *tuple = cast_type<TupleType>(selfRef.get())) {
            element = tuple->elementType();
        } else {
            ENFORCE(false, "Array#flatten on unexpected type: ", selfRef->show(ctx));
        }
        return Types::arrayOf(ctx, recursivelyFlattenArrays(ctx, element));
    }
} Array_flatten;

class Array_compact : public Symbol::IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, Loc callLoc, vector<TypeAndOrigins> &args, shared_ptr<Type> selfRef,
                           shared_ptr<Type> fullType, shared_ptr<SendAndBlockLink> linkType) const override {
        shared_ptr<Type> element;
        if (auto *ap = cast_type<AppliedType>(selfRef.get())) {
            ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(ctx).derivesFrom(ctx, Symbols::Array()));
            ENFORCE(!ap->targs.empty());
            element = ap->targs.front();
        } else if (auto *tuple = cast_type<TupleType>(selfRef.get())) {
            element = tuple->elementType();
        } else {
            ENFORCE(false, "Array#compact on unexpected type: ", selfRef->show(ctx));
        }
        auto ret = Types::approximateSubtract(ctx, element, Types::nilClass());
        return Types::arrayOf(ctx, ret);
    }
} Array_compact;
} // namespace

const vector<Intrinsic> intrinsicMethods{
    {Symbols::T(), true, Names::untyped(), &T_untyped},
    {Symbols::T(), true, Names::must(), &T_must},
    {Symbols::T(), true, Names::all(), &T_all},
    {Symbols::T(), true, Names::any(), &T_any},
    {Symbols::T(), true, Names::revealType(), &T_revealType},

    {Symbols::T_Generic(), false, Names::squareBrackets(), &T_Generic_squareBrackets},

    {Symbols::T_Array(), true, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Hash(), true, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Enumerable(), true, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Range(), true, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Set(), true, Names::squareBrackets(), &T_Generic_squareBrackets},

    {Symbols::Object(), false, Names::class_(), &Object_class},
    {Symbols::Object(), false, Names::singletonClass(), &Object_class},

    {Symbols::Class(), false, Names::new_(), &Class_new},

    {Symbols::Magic(), false, Names::buildHash(), &Magic_buildHash},
    {Symbols::Magic(), false, Names::buildArray(), &Magic_buildArray},
    {Symbols::Magic(), false, Names::expandSplat(), &Magic_expandSplat},

    {Symbols::Tuple(), false, Names::squareBrackets(), &Tuple_squareBrackets},
    {Symbols::Tuple(), false, Names::first(), &Tuple_first},
    {Symbols::Tuple(), false, Names::last(), &Tuple_last},
    {Symbols::Tuple(), false, Names::min(), &Tuple_minMax},
    {Symbols::Tuple(), false, Names::max(), &Tuple_minMax},

    {Symbols::Array(), false, Names::flatten(), &Array_flatten},
    {Symbols::Array(), false, Names::compact(), &Array_compact},
};

} // namespace core
} // namespace sorbet
