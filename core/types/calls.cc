#include "common/common.h"
#include "core/GlobalState.h"
#include "core/Names/core.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"
#include "core/errors/infer.h"
#include <algorithm> // find_if, sort

#include "../Types.h"
#include "absl/algorithm/container.h"
#include "absl/strings/str_cat.h"

template class std::vector<sorbet::core::SymbolRef>;
using namespace std;

namespace sorbet {
namespace core {

DispatchResult ProxyType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "proxytype");
    auto und = underlying();
    return und->dispatchCall(ctx, args);
}

shared_ptr<Type> ProxyType::getCallArgumentType(Context ctx, NameRef name, int i) {
    return underlying()->getCallArgumentType(ctx, name, i);
}

DispatchResult OrType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "ortype");
    auto leftRet = left->dispatchCall(ctx, args.withSelfRef(left));
    auto rightRet = right->dispatchCall(ctx, args.withSelfRef(right));
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

DispatchResult TypeVar::dispatchCall(Context ctx, DispatchArgs args) {
    Error::raise("should never happen");
}

DispatchResult AndType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "andtype");
    auto leftRet = left->dispatchCall(ctx, args);
    auto rightRet = right->dispatchCall(ctx, args);

    // If either side is missing the method, dispatch to the other.
    auto leftOk = absl::c_all_of(leftRet.components, [&](auto &comp) { return comp.method.exists(); });
    auto rightOk = absl::c_all_of(rightRet.components, [&](auto &comp) { return comp.method.exists(); });
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

DispatchResult ShapeType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "shapetype");
    auto method = Symbols::Shape().data(ctx).findMember(ctx, args.name);
    if (method.exists() && method.data(ctx).intrinsic != nullptr) {
        auto result = method.data(ctx).intrinsic->apply(ctx, args, this);
        if (result != nullptr) {
            DispatchResult::ComponentVec components(1);
            components.front().receiver = args.selfType;
            components.front().method = method;
            return DispatchResult{result, move(components)};
        }
    }
    return ProxyType::dispatchCall(ctx, args);
}

DispatchResult TupleType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "tupletype");
    auto method = Symbols::Tuple().data(ctx).findMember(ctx, args.name);
    if (method.exists() && method.data(ctx).intrinsic != nullptr) {
        auto result = method.data(ctx).intrinsic->apply(ctx, args, this);
        if (result != nullptr) {
            DispatchResult::ComponentVec components(1);
            components.front().receiver = args.selfType;
            components.front().method = method;
            return DispatchResult{result, move(components)};
        }
    }
    return ProxyType::dispatchCall(ctx, args);
}

namespace {
bool isSetter(Context ctx, NameRef fun) {
    if (fun.data(ctx).kind != NameKind::UTF8) {
        return false;
    }
    return fun.data(ctx).raw.utf8.back() == '=';
}

unique_ptr<BasicError> matchArgType(Context ctx, TypeConstraint &constr, Loc callLoc, Loc receiverLoc,
                                    SymbolRef inClass, SymbolRef method, const TypeAndOrigins &argTpe,
                                    const Symbol &argSym, const shared_ptr<Type> &selfType,
                                    vector<shared_ptr<Type>> &targs, Loc loc, bool mayBeSetter = false) {
    auto ref = argSym.ref(ctx);
    shared_ptr<Type> expectedType = Types::resultTypeAsSeenFrom(ctx, ref, inClass, targs);
    if (!expectedType) {
        expectedType = Types::untyped(ctx, ref);
    }

    expectedType = Types::replaceSelfType(ctx, expectedType, selfType);

    if (Types::isSubTypeUnderConstraint(ctx, constr, argTpe.type, expectedType)) {
        return nullptr;
    }
    if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentMismatch)) {
        if (mayBeSetter && isSetter(ctx, method.data(ctx).name)) {
            e.setHeader("Assigning a value to `{}` that does not match expected type `{}`", argSym.name.toString(ctx),
                        expectedType->show(ctx));
        } else {
            e.setHeader("`{}` doesn't match `{}` for argument `{}`", argTpe.type->show(ctx), expectedType->show(ctx),
                        argSym.name.toString(ctx));
            e.addErrorSection(ErrorSection({
                ErrorLine::from(argSym.loc(), "Method `{}` has specified `{}` as `{}`", method.data(ctx).show(ctx),
                                argSym.name.toString(ctx), expectedType->show(ctx)),
            }));
        }
        e.addErrorSection(
            ErrorSection("Got " + argTpe.type->show(ctx) + " originating from:", argTpe.origins2Explanations(ctx)));
        auto withoutNil = Types::approximateSubtract(ctx, argTpe.type, Types::nilClass());
        if (!withoutNil->isBottom() && Types::isSubTypeUnderConstraint(ctx, constr, withoutNil, expectedType)) {
            if (loc.exists()) {
                e.replaceWith(loc, "T.must({})", loc.source(ctx));
            }
        }
        return e.build();
    }
    return nullptr;
}

unique_ptr<BasicError> missingArg(Context ctx, Loc callLoc, Loc receiverLoc, SymbolRef method, SymbolRef arg) {
    if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentCountMismatch)) {
        e.setHeader("Missing required keyword argument `{}` for method `{}`", arg.data(ctx).name.toString(ctx),
                    method.data(ctx).show(ctx));
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
SymbolRef guessOverload(Context ctx, SymbolRef inClass, SymbolRef primary,
                        InlinedVector<const TypeAndOrigins *, 2> &args, const shared_ptr<Type> &fullType,
                        vector<shared_ptr<Type>> &targs, bool hasBlock) {
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

        absl::c_sort(allCandidates, [&](SymbolRef s1, SymbolRef s2) -> bool {
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
                if (argType->isFullyDefined() && !Types::isSubType(ctx, arg->type, argType)) {
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

        auto er = absl::c_equal_range(leftCandidates, args.size(), cmp);
        if (er.first != leftCandidates.end()) {
            leftCandidates.erase(leftCandidates.begin(), er.first);
        }
    }

    if (!leftCandidates.empty()) {
        return leftCandidates[0];
    }
    return fallback;
}

shared_ptr<Type> unwrapType(Context ctx, Loc loc, const shared_ptr<Type> &tp) {
    if (auto *metaType = cast_type<MetaType>(tp.get())) {
        return metaType->wrapped;
    }
    if (auto *classType = cast_type<ClassType>(tp.get())) {
        SymbolRef attachedClass = classType->symbol.data(ctx).attachedClass(ctx);
        if (!attachedClass.exists()) {
            if (auto e = ctx.state.beginError(loc, errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
            }
            return Types::untypedUntracked();
        }

        return attachedClass.data(ctx).externalType(ctx);
    }

    if (auto *shapeType = cast_type<ShapeType>(tp.get())) {
        vector<shared_ptr<Type>> unwrappedValues;
        for (auto value : shapeType->values) {
            unwrappedValues.emplace_back(unwrapType(ctx, loc, value));
        }
        return make_shared<ShapeType>(Types::hashOfUntyped(), shapeType->keys, unwrappedValues);
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
        return Types::untypedUntracked();
    }
    return tp;
}

string prettyArity(Context ctx, SymbolRef method) {
    int required = 0, optional = 0;
    bool repeated = false;
    for (auto arg : method.data(ctx).arguments()) {
        if (arg.data(ctx).isKeyword() || arg.data(ctx).isBlockArgument()) {
            // ignore
        } else if (arg.data(ctx).isOptional()) {
            ++optional;
        } else if (arg.data(ctx).isRepeated()) {
            repeated = true;
        } else {
            ++required;
        }
    }
    if (repeated) {
        return absl::StrCat(required, "+");
    } else if (optional > 0) {
        return absl::StrCat(required, "..", required + optional);
    } else {
        return to_string(required);
    }
}

// This implements Ruby's argument matching logic (assigning values passed to a
// method call to formal parameters of the method).
//
// Known incompleteness or inconsistencies with Ruby:
//  - Missing coercion to keyword arguments via `#to_hash`
//  - We never allow a non-shaped Hash to satisfy keyword arguments;
//    We should, at a minimum, probably allow one to satisfy an **kwargs : untyped
//    (with a subtype check on the key type, once we have generics)
DispatchResult dispatchCallSymbol(Context ctx, DispatchArgs args,

                                  const Type *thisType, core::SymbolRef symbol, vector<shared_ptr<Type>> &targs) {
    if (symbol == core::Symbols::untyped()) {
        return DispatchResult(Types::untyped(ctx, thisType->untypedBlame()), move(args.selfType), Symbols::untyped());
    } else if (symbol == Symbols::void_()) {
        if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::UnknownMethod)) {
            e.setHeader("Can not call method `{}` on void type", args.name.data(ctx).toString(ctx));
        }
        return DispatchResult(Types::untypedUntracked(), move(args.selfType), Symbols::noSymbol());
    }

    SymbolRef mayBeOverloaded = symbol.data(ctx).findMemberTransitive(ctx, args.name);

    if (!mayBeOverloaded.exists()) {
        if (args.name == Names::initialize()) {
            // Special-case initialize(). We should define this on
            // `BasicObject`, but our method-resolution order is wrong, and
            // putting it there will inadvertently shadow real definitions in
            // some cases, so we special-case it here as a last resort.
            auto result = DispatchResult(Types::untypedUntracked(), move(args.selfType), Symbols::noSymbol());
            if (!args.args.empty()) {
                if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Wrong number of arguments for constructor. Expected: `{}`, got: `{}`", 0,
                                args.args.size());
                    result.components.front().errors.emplace_back(e.build());
                }
            }
            return result;
        }
        auto result = DispatchResult(Types::untypedUntracked(), move(args.selfType), Symbols::noSymbol());
        if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::UnknownMethod)) {
            if (args.fullType.get() != thisType) {
                e.setHeader("Method `{}` does not exist on `{}` component of `{}`", args.name.data(ctx).toString(ctx),
                            thisType->show(ctx), args.fullType->show(ctx));
            } else {
                e.setHeader("Method `{}` does not exist on `{}`", args.name.data(ctx).toString(ctx),
                            thisType->show(ctx));
            }
            if (args.fullType.get() != thisType && symbol == Symbols::NilClass()) {
                e.replaceWith(args.locs.receiver, "T.must({})", args.locs.receiver.source(ctx));
            } else {
                if (symbol.data(ctx).isClassModule()) {
                    auto objMeth = core::Symbols::Object().data(ctx).findMemberTransitive(ctx, args.name);
                    if (objMeth.exists()) {
                        e.addErrorSection(
                            ErrorSection(ErrorColors::format("Did you mean to `include {}` in this module?",
                                                             objMeth.data(ctx).owner.data(ctx).name.show(ctx))));
                    }
                }
                auto alternatives = symbol.data(ctx).findMemberFuzzyMatch(ctx, args.name);
                if (!alternatives.empty()) {
                    vector<ErrorLine> lines;
                    for (auto alternative : alternatives) {
                        lines.emplace_back(ErrorLine::from(alternative.symbol.data(ctx).loc(), "Did you mean: `{}`?",
                                                           alternative.symbol.show(ctx)));
                    }
                    e.addErrorSection(ErrorSection(lines));
                }

                auto attached = symbol.data(ctx).attachedClass(ctx);
                if (attached.exists() && symbol.data(ctx).derivesFrom(ctx, Symbols::Chalk_Tools_Accessible())) {
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
                           ? guessOverload(ctx.withOwner(mayBeOverloaded), symbol, mayBeOverloaded, args.args,
                                           args.fullType, targs, args.block != nullptr)
                           : mayBeOverloaded;

    DispatchResult result;
    result.components.emplace_back(DispatchComponent{args.selfType, method, vector<unique_ptr<BasicError>>()});

    const Symbol &data = method.data(ctx);
    unique_ptr<TypeConstraint> maybeConstraint;
    TypeConstraint *constr;
    if (args.block) {
        constr = args.block->constr.get();
    } else if (data.isGenericMethod()) {
        maybeConstraint = make_unique<TypeConstraint>();
        constr = maybeConstraint.get();
    } else {
        constr = &TypeConstraint::EmptyFrozenConstraint;
    }

    if (data.isGenericMethod()) {
        constr->defineDomain(ctx, data.typeArguments());
    }
    bool hasKwargs = absl::c_any_of(data.arguments(), [&ctx](SymbolRef arg) { return arg.data(ctx).isKeyword(); });

    auto pit = data.arguments().begin();
    auto pend = data.arguments().end();

    if (pit != pend && (pend - 1)->data(ctx).isBlockArgument()) {
        --pend;
    }

    auto ait = args.args.begin();
    auto aend = args.args.end();

    while (pit != pend && ait != aend) {
        const Symbol &spec = pit->data(ctx);
        auto &arg = *ait;
        if (spec.isKeyword()) {
            break;
        }
        if (ait + 1 == aend && hasKwargs && arg->type->derivesFrom(ctx, Symbols::Hash()) &&
            (spec.isOptional() || spec.isRepeated())) {
            break;
        }

        auto offset = ait - args.args.begin();
        if (auto e = matchArgType(ctx, *constr, args.locs.call, args.locs.receiver, symbol, method, *arg, spec,
                                  args.selfType, targs, args.locs.args[offset], args.args.size() == 1)) {
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
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::MethodArgumentCountMismatch)) {
                e.setHeader("Not enough arguments provided for method `{}`. Expected: `{}`, got: `{}`", data.show(ctx),
                            prettyArity(ctx, method),
                            args.args.size()); // TODO: should use position and print the source tree, not the cfg one.
                e.addErrorLine(method.data(ctx).loc(), "`{}` defined here", data.show(ctx));
                if (args.name == core::Names::any() &&
                    symbol == core::Symbols::T().data(ctx).lookupSingletonClass(ctx)) {
                    e.addErrorSection(
                        core::ErrorSection("Hint: if you want to allow any type as an argument, use `T.untyped`"));
                }

                result.components.front().errors.emplace_back(e.build());
            }
        }
    }

    if (hasKwargs && ait != aend) {
        UnorderedSet<NameRef> consumed;
        auto &hashArg = *(aend - 1);

        // find keyword arguments and advance `pend` before them; We'll walk
        // `kwit` ahead below
        auto kwit = pit;
        while (!kwit->data(ctx).isKeyword()) {
            kwit++;
        }
        pend = kwit;

        if (hashArg->type->isUntyped()) {
            // Allow an untyped arg to satisfy all kwargs
            --aend;
        } else if (auto *hash = cast_type<ShapeType>(hashArg->type.get())) {
            --aend;

            while (kwit != data.arguments().end()) {
                const Symbol &spec = kwit->data(ctx);
                if (spec.isBlockArgument()) {
                    break;
                } else if (spec.isRepeated()) {
                    for (auto it = hash->keys.begin(); it != hash->keys.end(); ++it) {
                        auto key = *it;
                        SymbolRef klass = cast_type<ClassType>(key->underlying().get())->symbol;
                        if (klass != Symbols::Symbol()) {
                            continue;
                        }

                        NameRef arg(ctx.state, key->value);
                        if (consumed.find(NameRef(ctx.state, key->value)) != consumed.end()) {
                            continue;
                        }
                        consumed.insert(arg);

                        TypeAndOrigins tpe;
                        tpe.origins = args.args.back()->origins;
                        auto offset = it - hash->keys.begin();
                        tpe.type = hash->values[offset];
                        if (auto e = matchArgType(ctx, *constr, args.locs.call, args.locs.receiver, symbol, method, tpe,
                                                  spec, args.selfType, targs, Loc::none())) {
                            result.components.front().errors.emplace_back(move(e));
                        }
                    }
                    break;
                }
                ++kwit;

                auto arg = absl::c_find_if(hash->keys, [&](shared_ptr<LiteralType> lit) {
                    return cast_type<ClassType>(lit->underlying().get())->symbol == Symbols::Symbol() &&
                           lit->value == spec.name._id;
                });
                if (arg == hash->keys.end()) {
                    if (!spec.isOptional()) {
                        if (auto e = missingArg(ctx, args.locs.call, args.locs.receiver, method, spec.ref(ctx))) {
                            result.components.front().errors.emplace_back(move(e));
                        }
                    }
                    continue;
                }
                consumed.insert(spec.name);
                TypeAndOrigins tpe;
                tpe.origins = args.args.back()->origins;
                auto offset = arg - hash->keys.begin();
                tpe.type = hash->values[offset];
                if (auto e = matchArgType(ctx, *constr, args.locs.call, args.locs.receiver, symbol, method, tpe, spec,
                                          args.selfType, targs, Loc::none())) {
                    result.components.front().errors.emplace_back(move(e));
                }
            }
            for (auto &key : hash->keys) {
                SymbolRef klass = cast_type<ClassType>(key->underlying().get())->symbol;
                if (klass == Symbols::Symbol() && consumed.find(NameRef(ctx.state, key->value)) != consumed.end()) {
                    continue;
                }
                NameRef arg(ctx.state, key->value);

                if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Unrecognized keyword argument `{}` passed for method `{}`", arg.toString(ctx),
                                data.show(ctx));
                    result.components.front().errors.emplace_back(e.build());
                }
            }
        } else if (hashArg->type->derivesFrom(ctx, Symbols::Hash())) {
            --aend;
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::MethodArgumentMismatch)) {
                e.setHeader("Passing an untyped hash to keyword arguments");
                e.addErrorSection(ErrorSection("Got " + hashArg->type->show(ctx) + " originating from:",
                                               hashArg->origins2Explanations(ctx)));
                result.components.front().errors.emplace_back(e.build());
            }
        }
    }
    if (hasKwargs && aend == args.args.end()) {
        // We have keyword arguments, but we didn't consume a hash at the
        // end. Report an error for each missing required keyword arugment.
        for (auto &spec : data.arguments()) {
            if (!spec.data(ctx).isKeyword() || spec.data(ctx).isOptional() || spec.data(ctx).isRepeated()) {
                continue;
            }
            if (auto e = missingArg(ctx, args.locs.call, args.locs.receiver, method, spec)) {
                result.components.front().errors.emplace_back(move(e));
            }
        }
    }

    if (ait != aend) {
        if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::MethodArgumentCountMismatch)) {
            e.setHeader("Too many arguments provided for method `{}`. Expected: `{}`, got: `{}`", data.show(ctx),
                        prettyArity(ctx, method), args.args.size());
            e.addErrorLine(method.data(ctx).loc(), "`{}` defined here", args.name.toString(ctx));
            result.components.front().errors.emplace_back(e.build());
        }
    }

    shared_ptr<Type> resultType = nullptr;

    if (method.data(ctx).intrinsic != nullptr) {
        resultType = method.data(ctx).intrinsic->apply(ctx, args, thisType);
    }

    if (resultType == nullptr) {
        resultType = Types::resultTypeAsSeenFrom(ctx, method, symbol, targs);
    }
    if (args.block == nullptr) {
        // if block is there we do not attempt to solve the constaint. CFG adds an explicit solve
        // node that triggers constraint solving
        if (!constr->solve(ctx)) {
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::GenericMethodConstaintUnsolved)) {
                e.setHeader("Could not find valid instantiation of type parameters");
                result.components.front().errors.emplace_back(e.build());
            }
        }
    }

    if (!resultType) {
        resultType = Types::untyped(ctx, method);
    } else if (!constr->isEmpty() && constr->isSolved()) {
        resultType = Types::instantiate(ctx, resultType, *constr);
    }
    resultType = Types::replaceSelfType(ctx, resultType, args.selfType);

    if (args.block != nullptr) {
        SymbolRef bspec;
        if (!data.arguments().empty()) {
            bspec = data.arguments().back();
        }
        if (!bspec.exists() || !bspec.data(ctx).isBlockArgument()) {
            // TODO(nelhage): passing a block to a function that does not accept one
        } else {
            shared_ptr<Type> blockType = Types::resultTypeAsSeenFrom(ctx, bspec, symbol, targs);
            if (!blockType) {
                blockType = Types::untyped(ctx, bspec);
            }

            args.block->returnTp = Types::getProcReturnType(ctx, blockType);
            blockType = constr->isSolved() ? Types::instantiate(ctx, blockType, *constr)
                                           : Types::approximate(ctx, blockType, *constr);

            args.block->blockPreType = blockType;
            args.block->sendTp = resultType;
        }
    }
    result.returnType = move(resultType);
    return result;
}

DispatchResult ClassType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "classtype");
    vector<shared_ptr<Type>> empty;
    return dispatchCallSymbol(ctx, args, this, symbol, empty);
}

DispatchResult AppliedType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "appliedType");
    return dispatchCallSymbol(ctx, args, this, this->klass, this->targs);
}

shared_ptr<Type> ClassType::getCallArgumentType(Context ctx, NameRef name, int i) {
    if (isUntyped()) {
        return Types::untyped(ctx, untypedBlame());
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
        resultType = Types::untyped(ctx, data.arguments()[i]);
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
        resultType = Types::untyped(ctx, data.arguments()[i]);
    }
    return resultType;
}

DispatchResult AliasType::dispatchCall(Context ctx, DispatchArgs args) {
    Error::raise("AliasType::dispatchCall");
}

shared_ptr<Type> AliasType::getCallArgumentType(Context ctx, NameRef name, int i) {
    Error::raise("AliasType::getCallArgumentType");
}

DispatchResult MetaType::dispatchCall(Context ctx, DispatchArgs args) {
    switch (args.name._id) {
        case Names::new_()._id: {
            auto res =
                DispatchResult(wrapped, args.selfType, Symbols::Class().data(ctx).findMember(ctx, Names::new_()));
            auto innerArgs = DispatchArgs{Names::initialize(), args.locs, args.args, wrapped, wrapped, args.block};
            auto inner = wrapped->dispatchCall(ctx, innerArgs);
            res.components.insert(res.components.end(), make_move_iterator(inner.components.begin()),
                                  make_move_iterator(inner.components.end()));
            return res;
        }
        default:
            return ProxyType::dispatchCall(ctx, args);
    }
}

SymbolRef unwrapSymbol(const Type *type) {
    SymbolRef result;
    while (!result.exists()) {
        typecase(type,

                 [&](const ClassType *klass) { result = klass->symbol; },

                 [&](const AppliedType *app) { result = app->klass; },

                 [&](const ProxyType *proxy) { type = proxy->underlying().get(); },

                 [&](const Type *ty) { ENFORCE(false, "Unexpected type: ", ty->typeName()); });
    }
    return result;
}
namespace {

class T_untyped : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        return Types::untypedUntracked();
    }
} T_untyped;

class T_must : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.empty()) {
            return nullptr;
        }
        if (!args.args[0]->type->isFullyDefined()) {
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::BareTypeUsage)) {
                e.setHeader("T.must() applied to incomplete type `{}`", args.args[0]->type->show(ctx));
            }
            return nullptr;
        }
        auto ret = Types::approximateSubtract(ctx, args.args[0]->type, Types::nilClass());
        if (ret == args.args[0]->type) {
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::InvalidCast)) {
                e.setHeader("T.must(): Expected a `T.nilable` type, got: `{}`", args.args[0]->type->show(ctx));
            }
        }
        return ret;
    }
} T_must;

class T_any : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.empty()) {
            return Types::untypedUntracked();
        }

        shared_ptr<Type> res = Types::bottom();
        auto i = -1;
        for (auto &arg : args.args) {
            i++;
            auto ty = unwrapType(ctx, args.locs.args[i], arg->type);
            res = Types::any(ctx, res, ty);
        }

        return make_shared<MetaType>(res);
    }
} T_any;

class T_all : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.empty()) {
            return Types::untypedUntracked();
        }

        shared_ptr<Type> res = Types::top();
        auto i = -1;
        for (auto &arg : args.args) {
            i++;
            auto ty = unwrapType(ctx, args.locs.args[i], arg->type);
            res = Types::all(ctx, res, ty);
        }

        return make_shared<MetaType>(res);
    }
} T_all;

class T_revealType : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.size() != 1) {
            return Types::untypedUntracked();
        }

        if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::RevealType)) {
            e.setHeader("Revealed type: `{}`", args.args[0]->type->show(ctx));
            e.addErrorSection(ErrorSection("From:", args.args[0]->origins2Explanations(ctx)));
        }
        return args.args[0]->type;
    }
} T_revealType;

class T_nilable : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.size() != 1) {
            return Types::untypedUntracked();
        }

        return make_shared<MetaType>(
            Types::any(ctx, unwrapType(ctx, args.locs.args[0], args.args[0]->type), Types::nilClass()));
    }
} T_nilable;

class Object_class : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        SymbolRef self = unwrapSymbol(thisType);
        auto singleton = self.data(ctx).lookupSingletonClass(ctx);
        if (singleton.exists()) {
            return make_shared<ClassType>(singleton);
        }
        return Types::classClass();
    }
} Object_class;

class Class_new : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        SymbolRef self = unwrapSymbol(thisType);

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
        DispatchArgs innerArgs{Names::initialize(), args.locs, args.args, instanceTy, instanceTy, args.block};
        auto dispatched = instanceTy->dispatchCall(ctx, innerArgs);

        // This dispatch call will set return type in linkType to result of initialize.
        // Need to override it
        if (args.block) {
            args.block->sendTp = instanceTy;
        }

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

class T_Generic_squareBrackets : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        SymbolRef attachedClass;

        SymbolRef self = unwrapSymbol(thisType);
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

        if (args.args.size() != arity) {
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::GenericArgumentCountMismatch)) {
                e.setHeader("Wrong number of type parameters for `{}`. Expected: `{}`, got: `{}`",
                            attachedClass.data(ctx).show(ctx), arity, args.args.size());
            }
        }

        vector<shared_ptr<Type>> targs;
        auto it = args.args.begin();
        int i = -1;
        for (auto mem : attachedClass.data(ctx).typeMembers()) {
            ++i;
            if (mem.data(ctx).isFixed()) {
                targs.emplace_back(mem.data(ctx).resultType);
            } else if (it != args.args.end()) {
                targs.emplace_back(unwrapType(ctx, args.locs.args[it - args.args.begin()], (*it)->type));
                ++it;
            } else if (attachedClass == Symbols::Hash() && i == 2) {
                auto tupleArgs = targs;
                targs.emplace_back(TupleType::build(ctx, tupleArgs));
            } else {
                targs.emplace_back(Types::untypedUntracked());
            }
        }

        return make_shared<MetaType>(make_shared<AppliedType>(attachedClass, targs));
    }
} T_Generic_squareBrackets;

class Magic_buildHash : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        ENFORCE(args.args.size() % 2 == 0);

        vector<shared_ptr<LiteralType>> keys;
        vector<shared_ptr<Type>> values;
        for (int i = 0; i < args.args.size(); i += 2) {
            auto *key = cast_type<LiteralType>(args.args[i]->type.get());
            if (key == nullptr) {
                return Types::hashOfUntyped();
            }

            keys.push_back(shared_ptr<LiteralType>(args.args[i]->type, key));
            values.push_back(args.args[i + 1]->type);
        }
        return make_unique<ShapeType>(Types::hashOfUntyped(), keys, values);
    }
} Magic_buildHash;

class Magic_buildArray : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.empty()) {
            return Types::arrayOfUntyped();
        }
        vector<shared_ptr<Type>> elems;
        bool isType = absl::c_any_of(args.args, [ctx](auto ty) { return isa_type<MetaType>(ty->type.get()); });
        int i = -1;
        for (auto &elem : args.args) {
            ++i;
            if (isType) {
                elems.push_back(unwrapType(ctx, args.locs.args[i], elem->type));
            } else {
                elems.push_back(elem->type);
            }
        }

        auto tuple = TupleType::build(ctx, elems);
        if (isType) {
            return make_shared<MetaType>(tuple);
        } else {
            return tuple;
        }
    }
} Magic_buildArray;

class Magic_expandSplat : public IntrinsicMethod {
    static shared_ptr<Type> expandArray(Context ctx, const shared_ptr<Type> &type, int expandTo) {
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
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.size() != 3) {
            return Types::arrayOfUntyped();
        }
        auto val = args.args.front()->type;
        auto *beforeLit = cast_type<LiteralType>(args.args[1]->type.get());
        auto *afterLit = cast_type<LiteralType>(args.args[2]->type.get());
        if (!(beforeLit->underlying()->derivesFrom(ctx, Symbols::Integer()) &&
              afterLit->underlying()->derivesFrom(ctx, Symbols::Integer()))) {
            return Types::untypedUntracked();
        }
        int before = (int)beforeLit->value;
        int after = (int)afterLit->value;
        return expandArray(ctx, val, before + after);
    }
} Magic_expandSplat;

class Magic_callWithSplat : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.size() != 3) {
            return core::Types::untypedUntracked();
        }
        auto &receiver = args.args[0];
        if (receiver->type->isUntyped()) {
            return receiver->type;
        }

        auto *lit = cast_type<LiteralType>(args.args[1]->type.get());
        if (!lit || !lit->derivesFrom(ctx, Symbols::Symbol())) {
            return core::Types::untypedUntracked();
        }
        NameRef fn(ctx.state, (u4)lit->value);
        if (args.args[2]->type->isUntyped()) {
            return args.args[2]->type;
        }
        auto *tuple = cast_type<TupleType>(args.args[2]->type.get());
        if (tuple == nullptr) {
            if (auto e = ctx.state.beginError(args.locs.args[2], core::errors::Infer::UntypedSplat)) {
                e.setHeader("Splats are only supported where the size of the array is known statically");
            }
            return Types::untypedUntracked();
        }

        InlinedVector<TypeAndOrigins, 2> sendArgStore;
        for (auto &arg : tuple->elems) {
            TypeAndOrigins tao;
            tao.type = arg;
            tao.origins.emplace_back(args.locs.args[2]);
            sendArgStore.push_back(move(tao));
        }
        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        for (auto &arg : sendArgStore) {
            sendArgs.push_back(&arg);
        }
        InlinedVector<Loc, 2> sendArgLocs(tuple->elems.size(), args.locs.args[2]);
        CallLocs sendLocs{args.locs.call, args.locs.args[0], sendArgLocs};
        DispatchArgs innerArgs{fn, sendLocs, sendArgs, receiver->type, receiver->type, args.block};
        auto dispatched = receiver->type->dispatchCall(ctx, innerArgs);
        for (auto &comp : dispatched.components) {
            for (auto &err : comp.errors) {
                ctx.state._error(move(err));
            }
        }
        return dispatched.returnType;
    }
} Magic_callWithSplat;

class Tuple_squareBrackets : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        auto *tuple = cast_type<TupleType>(thisType);
        ENFORCE(tuple);
        LiteralType *lit = nullptr;
        if (args.args.size() == 1) {
            lit = cast_type<LiteralType>(args.args.front()->type.get());
        }
        if (!lit || !lit->underlying()->derivesFrom(ctx, Symbols::Integer())) {
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

class Tuple_last : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        auto *tuple = cast_type<TupleType>(thisType);
        ENFORCE(tuple);

        if (!args.args.empty()) {
            return nullptr;
        }
        if (tuple->elems.empty()) {
            return Types::nilClass();
        }
        return tuple->elems.back();
    }
} Tuple_last;

class Tuple_first : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        auto *tuple = cast_type<TupleType>(thisType);
        ENFORCE(tuple);

        if (!args.args.empty()) {
            return nullptr;
        }
        if (tuple->elems.empty()) {
            return Types::nilClass();
        }
        return tuple->elems.front();
    }
} Tuple_first;

class Tuple_minMax : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        auto *tuple = cast_type<TupleType>(thisType);
        ENFORCE(tuple);

        if (!args.args.empty()) {
            return nullptr;
        }
        if (tuple->elems.empty()) {
            return Types::nilClass();
        }
        return tuple->elementType();
    }
} Tuple_minMax;

class Tuple_to_a : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        return args.selfType;
    }
} Tuple_to_a;

class Tuple_concat : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        std::vector<std::shared_ptr<Type>> elems;
        auto *tuple = cast_type<TupleType>(thisType);
        ENFORCE(tuple);
        elems = tuple->elems;
        for (auto elem : args.args) {
            if (auto *tuple = cast_type<TupleType>(elem->type.get())) {
                elems.insert(elems.end(), tuple->elems.begin(), tuple->elems.end());
            } else {
                return nullptr;
            }
        }
        return TupleType::build(ctx, move(elems));
    }
} Tuple_concat;

class Shape_merge : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        auto *shape = cast_type<ShapeType>(thisType);
        ENFORCE(shape);
        ShapeType *rhs = nullptr;
        if (!args.args.empty()) {
            rhs = cast_type<ShapeType>(args.args.front()->type.get());
        }
        if (rhs == nullptr || args.block != nullptr || args.args.size() > 1) {
            return nullptr;
        }

        auto keys = shape->keys;
        auto values = shape->values;
        for (auto &key : rhs->keys) {
            auto &value = rhs->values[&key - &rhs->keys.front()];
            auto fnd = absl::c_find_if(keys, [&key](auto &lit) { return key->equals(lit); });
            if (fnd == keys.end()) {
                keys.emplace_back(key);
                values.emplace_back(value);
            } else {
                values[fnd - keys.begin()] = value;
            }
        }

        return make_shared<ShapeType>(Types::hashOfUntyped(), move(keys), move(values));
    }
} Shape_merge;

class Array_flatten : public IntrinsicMethod {
    // Flattens a (nested) array all way down to its (inner) element type, stopping if we hit the depth limit first.
    static shared_ptr<Type> recursivelyFlattenArrays(Context ctx, const shared_ptr<Type> &type, const int64_t depth) {
        ENFORCE(type != nullptr);

        if (depth == 0) {
            return type;
        }
        const int newDepth = depth - 1;

        shared_ptr<Type> result;
        typecase(type.get(),

                 // This only shows up because t->elementType() for tuples returns an OrType of all its elements.
                 // So to properly handle nested tuples, we have to descend into the OrType's.
                 [&](OrType *o) {
                     result = Types::any(ctx, recursivelyFlattenArrays(ctx, o->left, newDepth),
                                         recursivelyFlattenArrays(ctx, o->right, newDepth));
                 },

                 [&](AppliedType *a) {
                     if (a->klass != Symbols::Array()) {
                         result = type;
                         return;
                     }
                     ENFORCE(a->targs.size() == 1);
                     result = recursivelyFlattenArrays(ctx, a->targs.front(), newDepth);
                 },

                 [&](TupleType *t) { result = recursivelyFlattenArrays(ctx, t->elementType(), newDepth); },

                 [&](Type *t) { result = move(type); });
        return result;
    }

public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        // Unwrap the array one time to get the element type (we'll rewrap it down at the bottom)
        shared_ptr<Type> element;
        if (auto *ap = cast_type<AppliedType>(thisType)) {
            ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(ctx).derivesFrom(ctx, Symbols::Array()));
            ENFORCE(!ap->targs.empty());
            element = ap->targs.front();
        } else if (auto *tuple = cast_type<TupleType>(thisType)) {
            element = tuple->elementType();
        } else {
            ENFORCE(false, "Array#flatten on unexpected type: ", args.selfType->show(ctx));
        }

        int64_t depth = INT64_MAX;
        if (args.args.size() == 1) {
            auto argTyp = args.args[0]->type;
            ENFORCE(args.locs.args.size() == 1,
                    "Mismatch between args.size() andargs.locs.args.size(): ", args.locs.args.size());
            auto argLoc = args.locs.args[0];

            auto lt = cast_type<LiteralType>(argTyp.get());
            if (!lt) {
                if (auto e = ctx.state.beginError(argLoc, core::errors::Infer::ExpectedLiteralType)) {
                    e.setHeader("You must pass an Integer literal to specify a depth with Array#flatten");
                }
                return Types::untypedUntracked();
            }
            ENFORCE(lt->literalKind == LiteralType::LiteralTypeKind::Integer, "depth arg must be an Integer literal");

            if (lt->value >= 0) {
                depth = lt->value;
            } else {
                // Negative values behave like no depth was given
                depth = INT64_MAX;
            }
        } else {
            ENFORCE(args.args.empty(), "Array#flatten passed too many args: ", args.args.size());
        }

        return Types::arrayOf(ctx, recursivelyFlattenArrays(ctx, element, depth));
    }
} Array_flatten;

class Array_compact : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        shared_ptr<Type> element;
        if (auto *ap = cast_type<AppliedType>(thisType)) {
            ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(ctx).derivesFrom(ctx, Symbols::Array()));
            ENFORCE(!ap->targs.empty());
            element = ap->targs.front();
        } else if (auto *tuple = cast_type<TupleType>(thisType)) {
            element = tuple->elementType();
        } else {
            ENFORCE(false, "Array#compact on unexpected type: ", args.selfType->show(ctx));
        }
        auto ret = Types::approximateSubtract(ctx, element, Types::nilClass());
        return Types::arrayOf(ctx, ret);
    }
} Array_compact;

class Kernel_proc : public IntrinsicMethod {
public:
    shared_ptr<Type> apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.block == nullptr) {
            return core::Types::untypedUntracked();
        }

        int arity = 0;
        for (auto arg : args.block->block.data(ctx).arguments()) {
            if (arg.data(ctx).isKeyword() || arg.data(ctx).isBlockArgument() || arg.data(ctx).isOptional() ||
                arg.data(ctx).isRepeated()) {
                return core::Types::procClass();
            }
            ++arity;
        }
        if (arity > core::Symbols::MAX_PROC_ARITY) {
            return core::Types::procClass();
        }
        vector<shared_ptr<core::Type>> targs(arity + 1, core::Types::untypedUntracked());
        auto procClass = core::Symbols::Proc(arity);
        return make_shared<core::AppliedType>(procClass, targs);
    }
} Kernel_proc;

} // namespace

const vector<Intrinsic> intrinsicMethods{
    {Symbols::T(), true, Names::untyped(), &T_untyped},
    {Symbols::T(), true, Names::must(), &T_must},
    {Symbols::T(), true, Names::all(), &T_all},
    {Symbols::T(), true, Names::any(), &T_any},
    {Symbols::T(), true, Names::nilable(), &T_nilable},
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
    {Symbols::Magic(), false, Names::callWithSplat(), &Magic_callWithSplat},

    {Symbols::Tuple(), false, Names::squareBrackets(), &Tuple_squareBrackets},
    {Symbols::Tuple(), false, Names::first(), &Tuple_first},
    {Symbols::Tuple(), false, Names::last(), &Tuple_last},
    {Symbols::Tuple(), false, Names::min(), &Tuple_minMax},
    {Symbols::Tuple(), false, Names::max(), &Tuple_minMax},
    {Symbols::Tuple(), false, Names::to_a(), &Tuple_to_a},
    {Symbols::Tuple(), false, Names::concat(), &Tuple_concat},

    {Symbols::Shape(), false, Names::merge(), &Shape_merge},

    {Symbols::Array(), false, Names::flatten(), &Array_flatten},
    {Symbols::Array(), false, Names::compact(), &Array_compact},

    {Symbols::Kernel(), false, Names::proc(), &Kernel_proc},
    {Symbols::Kernel(), false, Names::lambda(), &Kernel_proc},
};

} // namespace core
} // namespace sorbet
