#include "common/common.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"
#include "core/errors/infer.h"
#include <algorithm> // find_if, sort

#include "absl/strings/str_cat.h"

template class std::vector<sorbet::core::SymbolRef>;
using namespace std;

namespace sorbet::core {

DispatchResult ProxyType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "proxytype");
    auto und = underlying();
    return und->dispatchCall(ctx, args);
}

TypePtr ProxyType::getCallArguments(Context ctx, NameRef name) {
    return underlying()->getCallArguments(ctx, name);
}

DispatchResult OrType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "ortype");
    auto leftRet = left->dispatchCall(ctx, args.withSelfRef(left));
    auto rightRet = right->dispatchCall(ctx, args.withSelfRef(right));
    DispatchResult::ComponentVec components = std::move(leftRet.components);
    components.insert(components.end(), make_move_iterator(rightRet.components.begin()),
                      make_move_iterator(rightRet.components.end()));
    DispatchResult ret{
        Types::any(ctx, leftRet.returnType, rightRet.returnType),
        std::move(components),
    };
    return ret;
}

TypePtr OrType::getCallArguments(Context ctx, NameRef name) {
    return left->getCallArguments(ctx, name); // TODO: should glb with right
}

DispatchResult TypeVar::dispatchCall(Context ctx, DispatchArgs args) {
    Exception::raise("should never happen");
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

    DispatchResult::ComponentVec components = std::move(leftRet.components);
    components.insert(components.end(), make_move_iterator(rightRet.components.begin()),
                      make_move_iterator(rightRet.components.end()));
    DispatchResult ret{
        Types::all(ctx, leftRet.returnType, rightRet.returnType),
        std::move(components),
    };
    return ret;
}

TypePtr AndType::getCallArguments(Context ctx, NameRef name) {
    auto l = left->getCallArguments(ctx, name);
    auto r = right->getCallArguments(ctx, name);
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
    auto method = Symbols::Shape().data(ctx)->findMember(ctx, args.name);
    if (method.exists() && method.data(ctx)->intrinsic != nullptr) {
        auto result = method.data(ctx)->intrinsic->apply(ctx, args, this);
        if (result != nullptr) {
            DispatchResult::ComponentVec components(1);
            components.front().receiver = args.selfType;
            components.front().method = method;
            return DispatchResult{result, std::move(components)};
        }
    }
    return ProxyType::dispatchCall(ctx, args);
}

DispatchResult TupleType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "tupletype");
    auto method = Symbols::Tuple().data(ctx)->findMember(ctx, args.name);
    if (method.exists() && method.data(ctx)->intrinsic != nullptr) {
        auto result = method.data(ctx)->intrinsic->apply(ctx, args, this);
        if (result != nullptr) {
            DispatchResult::ComponentVec components(1);
            components.front().receiver = args.selfType;
            components.front().method = method;
            return DispatchResult{result, std::move(components)};
        }
    }
    return ProxyType::dispatchCall(ctx, args);
}

namespace {
bool isSetter(Context ctx, NameRef fun) {
    if (fun.data(ctx)->kind != NameKind::UTF8) {
        return false;
    }
    const string_view rawName = fun.data(ctx)->raw.utf8;
    if (rawName.size() < 2) {
        return false;
    }
    if (rawName.back() == '=') {
        return !(fun == Names::leq() || fun == Names::geq() || fun == Names::tripleEq() || fun == Names::eqeq() ||
                 fun == Names::neq());
    }
    return false;
}

unique_ptr<Error> matchArgType(Context ctx, TypeConstraint &constr, Loc callLoc, Loc receiverLoc, SymbolRef inClass,
                               SymbolRef method, const TypeAndOrigins &argTpe, const SymbolData argSym,
                               const TypePtr &selfType, vector<TypePtr> &targs, Loc loc, bool mayBeSetter = false) {
    auto ref = argSym->ref(ctx);
    TypePtr expectedType = Types::resultTypeAsSeenFrom(ctx, ref, inClass, targs);
    if (!expectedType) {
        expectedType = Types::untyped(ctx, ref);
    }

    expectedType = Types::replaceSelfType(ctx, expectedType, selfType);

    if (Types::isSubTypeUnderConstraint(ctx, constr, argTpe.type, expectedType)) {
        return nullptr;
    }
    if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentMismatch)) {
        if (mayBeSetter && isSetter(ctx, method.data(ctx)->name)) {
            e.setHeader("Assigning a value to `{}` that does not match expected type `{}`", argSym->argumentName(ctx),
                        expectedType->show(ctx));
        } else {
            e.setHeader("`{}` doesn't match `{}` for argument `{}`", argTpe.type->show(ctx), expectedType->show(ctx),
                        argSym->argumentName(ctx));
            e.addErrorSection(ErrorSection({
                ErrorLine::from(argSym->loc(), "Method `{}` has specified `{}` as `{}`", method.data(ctx)->show(ctx),
                                argSym->argumentName(ctx), expectedType->show(ctx)),
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

unique_ptr<Error> missingArg(Context ctx, Loc callLoc, Loc receiverLoc, SymbolRef method, SymbolRef arg) {
    if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentCountMismatch)) {
        e.setHeader("Missing required keyword argument `{}` for method `{}`", arg.data(ctx)->name.show(ctx),
                    method.data(ctx)->show(ctx));
        return e.build();
    }
    return nullptr;
}
}; // namespace

int getArity(Context ctx, SymbolRef method) {
    ENFORCE(!method.data(ctx)->arguments().empty(), "Every method should have at least a block arg.");
    ENFORCE(method.data(ctx)->arguments().back().data(ctx)->isBlockArgument(), "Last arg should be the block arg.");

    // Don't count the block arg in the arity
    return method.data(ctx)->arguments().size() - 1;
}

// Guess overload. The way we guess is only arity based - we will return the overload that has the smallest number of
// arguments that is >= args.size()
SymbolRef guessOverload(Context ctx, SymbolRef inClass, SymbolRef primary,
                        InlinedVector<const TypeAndOrigins *, 2> &args, const TypePtr &fullType, vector<TypePtr> &targs,
                        bool hasBlock) {
    counterInc("calls.overloaded_invocations");
    ENFORCE(ctx.permitOverloadDefinitions(), "overload not permitted here");
    SymbolRef fallback = primary;
    vector<SymbolRef> allCandidates;

    allCandidates.emplace_back(primary);
    { // create candidates and sort them by number of arguments(stable by symbol id)
        int i = 0;
        SymbolRef current = primary;
        while (current.data(ctx)->isOverloaded()) {
            i++;
            NameRef overloadName = ctx.state.getNameUnique(UniqueNameKind::Overload, primary.data(ctx)->name, i);
            SymbolRef overload = primary.data(ctx)->owner.data(ctx)->findMember(ctx, overloadName);
            if (!overload.exists()) {
                Exception::raise("Corruption of overloads?");
            } else {
                allCandidates.emplace_back(overload);
                current = overload;
            }
        }

        fast_sort(allCandidates, [&](SymbolRef s1, SymbolRef s2) -> bool {
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

                auto argType = Types::resultTypeAsSeenFrom(ctx, candidate.data(ctx)->arguments()[i], inClass, targs);
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
            auto args = candidate.data(ctx)->arguments();
            ENFORCE(!args.empty(), "Should at least have a block argument.");
            auto mentionsBlockArg = !args.back().data(ctx)->isSyntheticBlockArgument();
            if (mentionsBlockArg != hasBlock) {
                it = leftCandidates.erase(it);
                continue;
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
} // namespace sorbet::core

TypePtr unwrapType(Context ctx, Loc loc, const TypePtr &tp) {
    if (auto *metaType = cast_type<MetaType>(tp.get())) {
        return metaType->wrapped;
    }
    if (auto *classType = cast_type<ClassType>(tp.get())) {
        SymbolRef attachedClass = classType->symbol.data(ctx)->attachedClass(ctx);
        if (!attachedClass.exists()) {
            if (auto e = ctx.state.beginError(loc, errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
            }
            return Types::untypedUntracked();
        }

        return attachedClass.data(ctx)->externalType(ctx);
    }

    if (auto *shapeType = cast_type<ShapeType>(tp.get())) {
        vector<TypePtr> unwrappedValues;
        unwrappedValues.reserve(shapeType->values.size());
        for (auto value : shapeType->values) {
            unwrappedValues.emplace_back(unwrapType(ctx, loc, value));
        }
        return make_type<ShapeType>(Types::hashOfUntyped(), shapeType->keys, unwrappedValues);
    } else if (auto *tupleType = cast_type<TupleType>(tp.get())) {
        vector<TypePtr> unwrappedElems;
        unwrappedElems.reserve(tupleType->elems.size());
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
    for (auto arg : method.data(ctx)->arguments()) {
        if (arg.data(ctx)->isKeyword() || arg.data(ctx)->isBlockArgument()) {
            // ignore
        } else if (arg.data(ctx)->isOptional()) {
            ++optional;
        } else if (arg.data(ctx)->isRepeated()) {
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

                                  const Type *thisType, core::SymbolRef symbol, vector<TypePtr> &targs) {
    if (symbol == core::Symbols::untyped()) {
        return DispatchResult(Types::untyped(ctx, thisType->untypedBlame()), std::move(args.selfType),
                              Symbols::untyped());
    } else if (symbol == Symbols::void_()) {
        if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::UnknownMethod)) {
            e.setHeader("Can not call method `{}` on void type", args.name.data(ctx)->show(ctx));
        }
        return DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noSymbol());
    }

    SymbolRef mayBeOverloaded = symbol.data(ctx)->findMemberTransitive(ctx, args.name);

    if (!mayBeOverloaded.exists()) {
        if (args.name == Names::initialize()) {
            // Special-case initialize(). We should define this on
            // `BasicObject`, but our method-resolution order is wrong, and
            // putting it there will inadvertently shadow real definitions in
            // some cases, so we special-case it here as a last resort.
            auto result = DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noSymbol());
            if (!args.args.empty()) {
                if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Wrong number of arguments for constructor. Expected: `{}`, got: `{}`", 0,
                                args.args.size());
                    result.components.front().errors.emplace_back(e.build());
                }
            }
            return result;
        } else if (args.name == core::Names::super()) {
            return DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::untyped());
        }
        auto result = DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noSymbol());
        if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::UnknownMethod)) {
            string thisStr = thisType->show(ctx);
            if (args.fullType.get() != thisType) {
                e.setHeader("Method `{}` does not exist on `{}` component of `{}`", args.name.data(ctx)->show(ctx),
                            thisType->show(ctx), args.fullType->show(ctx));
            } else {
                e.setHeader("Method `{}` does not exist on `{}`", args.name.data(ctx)->show(ctx), thisStr);

                // catch the special case of `interface!` or `abstract!` and
                // suggest adding `extend T::Helpers`.
                if (args.name == core::Names::declareInterface() || args.name == core::Names::declareAbstract()) {
                    e.addErrorSection(ErrorSection(ErrorColors::format(
                        "You may need to add `extend T::Helpers` to the definition of `{}`", thisStr)));
                }
            }
            if (args.fullType.get() != thisType && symbol == Symbols::NilClass()) {
                e.replaceWith(args.locs.receiver, "T.must({})", args.locs.receiver.source(ctx));
            } else {
                if (symbol.data(ctx)->isClassModule()) {
                    auto objMeth = core::Symbols::Object().data(ctx)->findMemberTransitive(ctx, args.name);
                    if (objMeth.exists()) {
                        e.addErrorSection(
                            ErrorSection(ErrorColors::format("Did you mean to `include {}` in this module?",
                                                             objMeth.data(ctx)->owner.data(ctx)->name.show(ctx))));
                    }
                }
                auto alternatives = symbol.data(ctx)->findMemberFuzzyMatch(ctx, args.name);
                if (!alternatives.empty()) {
                    vector<ErrorLine> lines;
                    lines.reserve(alternatives.size());
                    for (auto alternative : alternatives) {
                        auto possible_symbol = alternative.symbol.data(ctx);
                        if (!possible_symbol->isClass() && !possible_symbol->isMethod()) {
                            continue;
                        }
                        auto suggestedName = possible_symbol->isClass() ? alternative.symbol.show(ctx) + ".new"
                                                                        : alternative.symbol.show(ctx);
                        lines.emplace_back(
                            ErrorLine::from(alternative.symbol.data(ctx)->loc(), "Did you mean: `{}`?", suggestedName));
                    }
                    e.addErrorSection(ErrorSection(lines));
                }

                auto attached = symbol.data(ctx)->attachedClass(ctx);
                if (attached.exists() && symbol.data(ctx)->derivesFrom(ctx, Symbols::Chalk_Tools_Accessible())) {
                    e.addErrorSection(ErrorSection(
                        "If this method is generated by Chalk::Tools::Accessible, you "
                        "may need to re-generate the .rbi. Try running:\n" +
                        ErrorColors::format("  scripts/bin/remote-script sorbet/shim_generation/make_accessible.rb {}",
                                            attached.data(ctx)->showFullName(ctx))));
                }
            }
            result.components.front().errors.emplace_back(e.build());
        }
        return result;
    }

    SymbolRef method = mayBeOverloaded.data(ctx)->isOverloaded()
                           ? guessOverload(ctx.withOwner(mayBeOverloaded), symbol, mayBeOverloaded, args.args,
                                           args.fullType, targs, args.block != nullptr)
                           : mayBeOverloaded;

    DispatchResult result;
    result.components.emplace_back(DispatchComponent{args.selfType, method, vector<unique_ptr<Error>>()});

    const SymbolData data = method.data(ctx);
    unique_ptr<TypeConstraint> maybeConstraint;
    TypeConstraint *constr;
    if (args.block) {
        constr = args.block->constr.get();
    } else if (data->isGenericMethod()) {
        maybeConstraint = make_unique<TypeConstraint>();
        constr = maybeConstraint.get();
    } else {
        constr = &TypeConstraint::EmptyFrozenConstraint;
    }

    if (data->isGenericMethod()) {
        constr->defineDomain(ctx, data->typeArguments());
    }
    bool hasKwargs = absl::c_any_of(data->arguments(), [&ctx](SymbolRef arg) { return arg.data(ctx)->isKeyword(); });

    // p -> params, i.e., what was mentioned in the defintiion
    auto pit = data->arguments().begin();
    auto pend = data->arguments().end();

    ENFORCE(pit != pend, "Should at least have the block arg.");
    ENFORCE((pend - 1)->data(ctx)->isBlockArgument(),
            "Last arg should be the block arg: " + (pend - 1)->data(ctx)->show(ctx));
    // We'll type check the block arg separately from the rest of the args.
    --pend;

    // a -> args, i.e., what was passed at the call site
    auto ait = args.args.begin();
    auto aend = args.args.end();

    while (pit != pend && ait != aend) {
        const SymbolData spec = pit->data(ctx);
        auto &arg = *ait;
        if (spec->isKeyword()) {
            break;
        }
        if (ait + 1 == aend && hasKwargs && (spec->isOptional() || spec->isRepeated()) &&
            Types::approximate(ctx, arg->type, *constr)->derivesFrom(ctx, Symbols::Hash())) {
            break;
        }

        auto offset = ait - args.args.begin();
        if (auto e = matchArgType(ctx, *constr, args.locs.call, args.locs.receiver, symbol, method, *arg, spec,
                                  args.selfType, targs, args.locs.args[offset], args.args.size() == 1)) {
            result.components.front().errors.emplace_back(std::move(e));
        }

        if (!spec->isRepeated()) {
            ++pit;
        }
        ++ait;
    }

    if (pit != pend) {
        if (!(pit->data(ctx)->isKeyword() || pit->data(ctx)->isOptional() || pit->data(ctx)->isRepeated() ||
              pit->data(ctx)->isBlockArgument())) {
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::MethodArgumentCountMismatch)) {
                if (args.fullType.get() != thisType) {
                    e.setHeader(
                        "Not enough arguments provided for method `{}` on `{}` component of `{}`. Expected: `{}`, got: "
                        "`{}`",
                        data->show(ctx), thisType->show(ctx), args.fullType->show(ctx), prettyArity(ctx, method),
                        args.args.size()); // TODO: should use position and print the source tree, not the cfg one.
                } else {
                    e.setHeader(
                        "Not enough arguments provided for method `{}`. Expected: `{}`, got: `{}`", data->show(ctx),
                        prettyArity(ctx, method),
                        args.args.size()); // TODO: should use position and print the source tree, not the cfg one.
                }
                e.addErrorLine(method.data(ctx)->loc(), "`{}` defined here", data->show(ctx));
                if (args.name == core::Names::any() &&
                    symbol == core::Symbols::T().data(ctx)->lookupSingletonClass(ctx)) {
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
        auto hashArgType = Types::approximate(ctx, hashArg->type, *constr);

        // find keyword arguments and advance `pend` before them; We'll walk
        // `kwit` ahead below
        auto kwit = pit;
        while (!kwit->data(ctx)->isKeyword()) {
            kwit++;
        }
        pend = kwit;

        if (hashArgType->isUntyped()) {
            // Allow an untyped arg to satisfy all kwargs
            --aend;
        } else if (auto *hash = cast_type<ShapeType>(hashArgType.get())) {
            --aend;

            while (kwit != data->arguments().end()) {
                const SymbolData spec = kwit->data(ctx);
                if (spec->isBlockArgument()) {
                    break;
                } else if (spec->isRepeated()) {
                    for (auto it = hash->keys.begin(); it != hash->keys.end(); ++it) {
                        auto key = cast_type<LiteralType>(it->get());
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
                            result.components.front().errors.emplace_back(std::move(e));
                        }
                    }
                    break;
                }
                ++kwit;

                auto arg = absl::c_find_if(hash->keys, [&](const TypePtr &litType) {
                    auto lit = cast_type<LiteralType>(litType.get());
                    return cast_type<ClassType>(lit->underlying().get())->symbol == Symbols::Symbol() &&
                           lit->value == spec->name._id;
                });
                if (arg == hash->keys.end()) {
                    if (!spec->isOptional()) {
                        if (auto e = missingArg(ctx, args.locs.call, args.locs.receiver, method, spec->ref(ctx))) {
                            result.components.front().errors.emplace_back(std::move(e));
                        }
                    }
                    continue;
                }
                consumed.insert(spec->name);
                TypeAndOrigins tpe;
                tpe.origins = args.args.back()->origins;
                auto offset = arg - hash->keys.begin();
                tpe.type = hash->values[offset];
                if (auto e = matchArgType(ctx, *constr, args.locs.call, args.locs.receiver, symbol, method, tpe, spec,
                                          args.selfType, targs, Loc::none())) {
                    result.components.front().errors.emplace_back(std::move(e));
                }
            }
            for (auto &keyType : hash->keys) {
                auto key = cast_type<LiteralType>(keyType.get());
                SymbolRef klass = cast_type<ClassType>(key->underlying().get())->symbol;
                if (klass == Symbols::Symbol() && consumed.find(NameRef(ctx.state, key->value)) != consumed.end()) {
                    continue;
                }
                NameRef arg(ctx.state, key->value);

                if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Unrecognized keyword argument `{}` passed for method `{}`", arg.show(ctx),
                                data->show(ctx));
                    result.components.front().errors.emplace_back(e.build());
                }
            }
        } else if (hashArgType->derivesFrom(ctx, Symbols::Hash())) {
            --aend;
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::UntypedSplat)) {
                e.setHeader("Passing a hash where the specific keys are unknown to a method taking keyword arguments");
                e.addErrorSection(ErrorSection("Got " + hashArgType->show(ctx) + " originating from:",
                                               hashArg->origins2Explanations(ctx)));
                result.components.front().errors.emplace_back(e.build());
            }
        }
    }
    if (hasKwargs && aend == args.args.end()) {
        // We have keyword arguments, but we didn't consume a hash at the
        // end. Report an error for each missing required keyword arugment.
        for (auto &spec : data->arguments()) {
            if (!spec.data(ctx)->isKeyword() || spec.data(ctx)->isOptional() || spec.data(ctx)->isRepeated()) {
                continue;
            }
            if (auto e = missingArg(ctx, args.locs.call, args.locs.receiver, method, spec)) {
                result.components.front().errors.emplace_back(std::move(e));
            }
        }
    }

    if (ait != aend) {
        if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::MethodArgumentCountMismatch)) {
            e.setHeader("Too many arguments provided for method `{}`. Expected: `{}`, got: `{}`", data->show(ctx),
                        prettyArity(ctx, method), args.args.size());
            e.addErrorLine(method.data(ctx)->loc(), "`{}` defined here", args.name.show(ctx));
            result.components.front().errors.emplace_back(e.build());
        }
    }

    if (args.block != nullptr) {
        ENFORCE(!data->arguments().empty(), "Every symbol must at least have a block arg: {}", data->show(ctx));
        SymbolRef bspec = data->arguments().back();
        ENFORCE(bspec.data(ctx)->isBlockArgument(), "The last symbol must be the block arg: {}", data->show(ctx));

        TypePtr blockType = Types::resultTypeAsSeenFrom(ctx, bspec, symbol, targs);
        if (!blockType) {
            blockType = Types::untyped(ctx, bspec);
        }

        args.block->returnTp = Types::getProcReturnType(ctx, blockType);
        blockType = constr->isSolved() ? Types::instantiate(ctx, blockType, *constr)
                                       : Types::approximate(ctx, blockType, *constr);

        args.block->blockPreType = blockType;
        args.block->blockSpec = bspec;
    }

    TypePtr resultType = nullptr;

    if (method.data(ctx)->intrinsic != nullptr) {
        resultType = method.data(ctx)->intrinsic->apply(ctx, args, thisType);
    }

    if (resultType == nullptr) {
        if (args.args.size() == 1 && isSetter(ctx, method.data(ctx)->name)) {
            // assignments always return their right hand side
            resultType = args.args.front()->type;
        } else if (args.args.size() == 2 && method.data(ctx)->name == Names::squareBracketsEq()) {
            resultType = args.args[1]->type;
        } else {
            resultType = Types::resultTypeAsSeenFrom(ctx, method, symbol, targs);
        }
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
        ENFORCE(!data->arguments().empty(), "Every method should at least have a block arg.");
        ENFORCE(data->arguments().back().data(ctx)->isBlockArgument(), "The last arg should be the block arg.");
        auto blockType = data->arguments().back().data(ctx)->resultType;
        if (blockType && !core::Types::isSubType(ctx, core::Types::nilClass(), blockType)) {
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::BlockNotPassed)) {
                e.setHeader("`{}` requires a block parameter, but no block was passed", args.name.show(ctx));
                e.addErrorLine(method.data(ctx)->loc(), "defined here");
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
        args.block->sendTp = resultType;
    }
    result.returnType = std::move(resultType);
    return result;
}

DispatchResult ClassType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "classtype");
    vector<TypePtr> empty;
    return dispatchCallSymbol(ctx, args, this, symbol, empty);
}

DispatchResult AppliedType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "appliedType");
    return dispatchCallSymbol(ctx, args, this, this->klass, this->targs);
}

TypePtr getMethodArguments(Context ctx, SymbolRef klass, NameRef name, const vector<TypePtr> &targs) {
    SymbolRef method = klass.data(ctx)->findMemberTransitive(ctx, name);

    if (!method.exists()) {
        return nullptr;
    }
    const SymbolData data = method.data(ctx);

    vector<TypePtr> args;
    args.reserve(data->arguments().size());
    for (auto arg : data->arguments()) {
        if (arg.data(ctx)->isRepeated()) {
            ENFORCE(args.empty(), "getCallArguments with positional and repeated args is not supported: {}",
                    data->toString(ctx));
            return Types::arrayOf(ctx, Types::resultTypeAsSeenFrom(ctx, arg, klass, targs));
        }
        ENFORCE(!arg.data(ctx)->isKeyword(), "getCallArguments does not support kwargs: {}", data->toString(ctx));
        if (arg.data(ctx)->isBlockArgument()) {
            continue;
        }
        args.emplace_back(Types::resultTypeAsSeenFrom(ctx, arg, klass, targs));
    }
    return TupleType::build(ctx, args);
}

TypePtr ClassType::getCallArguments(Context ctx, NameRef name) {
    if (isUntyped()) {
        return Types::untyped(ctx, untypedBlame());
    }
    return getMethodArguments(ctx, symbol, name, vector<TypePtr>{});
}

TypePtr AppliedType::getCallArguments(Context ctx, NameRef name) {
    return getMethodArguments(ctx, klass, name, targs);
}

DispatchResult AliasType::dispatchCall(Context ctx, DispatchArgs args) {
    Exception::raise("AliasType::dispatchCall");
}

TypePtr AliasType::getCallArguments(Context ctx, NameRef name) {
    Exception::raise("AliasType::getCallArgumentType");
}

DispatchResult MetaType::dispatchCall(Context ctx, DispatchArgs args) {
    switch (args.name._id) {
        case Names::new_()._id: {
            auto res =
                DispatchResult(wrapped, args.selfType, Symbols::Class().data(ctx)->findMember(ctx, Names::new_()));
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
        typecase(
            type,

            [&](const ClassType *klass) { result = klass->symbol; },

            [&](const AppliedType *app) { result = app->klass; },

            [&](const ProxyType *proxy) { type = proxy->underlying().get(); },

            [&](const Type *ty) { ENFORCE(false, "Unexpected type: {}", ty->typeName()); });
    }
    return result;
}
namespace {

class T_untyped : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        return Types::untypedUntracked();
    }
} T_untyped;

class T_must : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
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
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.empty()) {
            return Types::untypedUntracked();
        }

        TypePtr res = Types::bottom();
        auto i = -1;
        for (auto &arg : args.args) {
            i++;
            auto ty = unwrapType(ctx, args.locs.args[i], arg->type);
            res = Types::any(ctx, res, ty);
        }

        return make_type<MetaType>(res);
    }
} T_any;

class T_all : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.empty()) {
            return Types::untypedUntracked();
        }

        TypePtr res = Types::top();
        auto i = -1;
        for (auto &arg : args.args) {
            i++;
            auto ty = unwrapType(ctx, args.locs.args[i], arg->type);
            res = Types::all(ctx, res, ty);
        }

        return make_type<MetaType>(res);
    }
} T_all;

class T_revealType : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
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
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.size() != 1) {
            return Types::untypedUntracked();
        }

        return make_type<MetaType>(
            Types::any(ctx, unwrapType(ctx, args.locs.args[0], args.args[0]->type), Types::nilClass()));
    }
} T_nilable;

class Object_class : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        SymbolRef self = unwrapSymbol(thisType);
        auto singleton = self.data(ctx)->lookupSingletonClass(ctx);
        if (singleton.exists()) {
            return make_type<ClassType>(singleton);
        }
        return Types::classClass();
    }
} Object_class;

class Class_new : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        SymbolRef self = unwrapSymbol(thisType);

        auto attachedClass = self.data(ctx)->attachedClass(ctx);
        if (!attachedClass.exists()) {
            if (self == Symbols::Class()) {
                // `Class.new(...)`, but it isn't a specific Class. We know
                // calling .new on a Class will yield some sort of Object
                attachedClass = Symbols::Object();
            } else {
                return nullptr;
            }
        }
        auto instanceTy = attachedClass.data(ctx)->externalType(ctx);
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
                ctx.state._error(std::move(err));
            }
        }
        return instanceTy;
    }
} Class_new;

class T_Generic_squareBrackets : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        SymbolRef attachedClass;

        SymbolRef self = unwrapSymbol(thisType);
        attachedClass = self.data(ctx)->attachedClass(ctx);

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

        auto arity = attachedClass.data(ctx)->typeArity(ctx);
        if (attachedClass == Symbols::Hash()) {
            arity = 2;
        }
        if (attachedClass.data(ctx)->typeMembers().empty()) {
            return nullptr;
        }

        if (args.args.size() != arity) {
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::GenericArgumentCountMismatch)) {
                e.setHeader("Wrong number of type parameters for `{}`. Expected: `{}`, got: `{}`",
                            attachedClass.data(ctx)->show(ctx), arity, args.args.size());
            }
        }

        vector<TypePtr> targs;
        auto it = args.args.begin();
        int i = -1;
        targs.reserve(attachedClass.data(ctx)->typeMembers().size());
        for (auto mem : attachedClass.data(ctx)->typeMembers()) {
            ++i;
            if (mem.data(ctx)->isFixed()) {
                targs.emplace_back(mem.data(ctx)->resultType);
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

        return make_type<MetaType>(make_type<AppliedType>(attachedClass, targs));
    }
} T_Generic_squareBrackets;

class Magic_buildHash : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        ENFORCE(args.args.size() % 2 == 0);

        vector<TypePtr> keys;
        vector<TypePtr> values;
        keys.reserve(args.args.size() / 2);
        values.reserve(args.args.size() / 2);
        for (int i = 0; i < args.args.size(); i += 2) {
            auto *key = cast_type<LiteralType>(args.args[i]->type.get());
            if (key == nullptr) {
                return Types::hashOfUntyped();
            }

            keys.emplace_back(args.args[i]->type);
            values.emplace_back(args.args[i + 1]->type);
        }
        return make_type<ShapeType>(Types::hashOfUntyped(), keys, values);
    }
} Magic_buildHash;

class Magic_buildArray : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.empty()) {
            return Types::arrayOfUntyped();
        }
        vector<TypePtr> elems;
        elems.reserve(args.args.size());
        bool isType = absl::c_any_of(args.args, [ctx](auto ty) { return isa_type<MetaType>(ty->type.get()); });
        int i = -1;
        for (auto &elem : args.args) {
            ++i;
            if (isType) {
                elems.emplace_back(unwrapType(ctx, args.locs.args[i], elem->type));
            } else {
                elems.emplace_back(elem->type);
            }
        }

        auto tuple = TupleType::build(ctx, elems);
        if (isType) {
            return make_type<MetaType>(tuple);
        } else {
            return tuple;
        }
    }
} Magic_buildArray;

class Magic_expandSplat : public IntrinsicMethod {
    static TypePtr expandArray(Context ctx, const TypePtr &type, int expandTo, core::TypeConstraint &constr) {
        if (auto *ot = cast_type<OrType>(type.get())) {
            return Types::any(ctx, expandArray(ctx, ot->left, expandTo, constr),
                              expandArray(ctx, ot->right, expandTo, constr));
        }

        auto *tuple = cast_type<TupleType>(type.get());
        if (tuple == nullptr && core::Types::approximate(ctx, type, constr)->derivesFrom(ctx, Symbols::Array())) {
            // If this is an array and not a tuple, just pass it through. We
            // can't say anything about the elements.
            return type;
        }
        vector<TypePtr> types;
        if (tuple) {
            types.insert(types.end(), tuple->elems.begin(), tuple->elems.end());
        } else {
            types.emplace_back(type);
        }
        if (types.size() < expandTo) {
            types.resize(expandTo, Types::nilClass());
        }

        return TupleType::build(ctx, types);
    }

public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
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
        return expandArray(ctx, val, before + after, args.constraint());
    }
} Magic_expandSplat;

class Magic_callWithSplat : public IntrinsicMethod {
    friend class Magic_callWithSplatAndBlock;

private:
    static InlinedVector<const TypeAndOrigins *, 2>
    generateSendArgs(TupleType *tuple, InlinedVector<TypeAndOrigins, 2> &sendArgStore, Loc argsLoc) {
        sendArgStore.reserve(tuple->elems.size());
        for (auto &arg : tuple->elems) {
            TypeAndOrigins tao;
            tao.type = arg;
            tao.origins.emplace_back(argsLoc);
            sendArgStore.emplace_back(std::move(tao));
        }
        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        sendArgs.reserve(sendArgStore.size());
        for (auto &arg : sendArgStore) {
            sendArgs.emplace_back(&arg);
        }

        return sendArgs;
    }

public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.args.size() != 3) {
            return core::Types::untypedUntracked();
        }
        auto &receiver = args.args[0];
        if (receiver->type->isUntyped()) {
            return receiver->type;
        }

        if (!receiver->type->isFullyDefined()) {
            return core::Types::untypedUntracked();
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
        InlinedVector<const TypeAndOrigins *, 2> sendArgs =
            Magic_callWithSplat::generateSendArgs(tuple, sendArgStore, args.locs.args[2]);
        InlinedVector<Loc, 2> sendArgLocs(tuple->elems.size(), args.locs.args[2]);
        CallLocs sendLocs{args.locs.call, args.locs.args[0], sendArgLocs};
        DispatchArgs innerArgs{fn, sendLocs, sendArgs, receiver->type, receiver->type, args.block};
        auto dispatched = receiver->type->dispatchCall(ctx, innerArgs);
        for (auto &comp : dispatched.components) {
            for (auto &err : comp.errors) {
                ctx.state._error(std::move(err));
            }
        }
        return dispatched.returnType;
    }
} Magic_callWithSplat;

class Magic_callWithBlock : public IntrinsicMethod {
    friend class Magic_callWithSplatAndBlock;

private:
    static TypePtr typeToProc(Context ctx, TypePtr blockType, Loc callLoc, Loc receiverLoc) {
        auto nonNilBlockType = blockType;
        auto typeIsNilable = false;
        if (Types::isSubType(ctx, Types::nilClass(), blockType)) {
            nonNilBlockType = Types::dropSubtypesOf(ctx, blockType, Symbols::NilClass());
            typeIsNilable = true;

            if (nonNilBlockType->isBottom()) {
                return Types::nilClass();
            }
        }

        NameRef to_proc = core::Names::to_proc();
        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        InlinedVector<Loc, 2> sendArgLocs;
        CallLocs sendLocs{callLoc, receiverLoc, sendArgLocs};
        DispatchArgs innerArgs{to_proc, sendLocs, sendArgs, nonNilBlockType, nonNilBlockType, nullptr};
        auto dispatched = nonNilBlockType->dispatchCall(ctx, innerArgs);
        for (auto &comp : dispatched.components) {
            for (auto &err : comp.errors) {
                ctx.state._error(std::move(err));
            }
        }

        if (typeIsNilable) {
            return Types::any(ctx, dispatched.returnType, Types::nilClass());
        } else {
            return dispatched.returnType;
        }
    }

    static std::optional<int> getArityForBlock(TypePtr blockType) {
        if (AppliedType *appliedType = cast_type<AppliedType>(blockType.get())) {
            return Types::getProcArity(*appliedType);
        }

        return std::nullopt;
    }

    static void showLocationOfArgDefn(Context ctx, ErrorBuilder &e, TypePtr blockType,
                                      DispatchComponent &dispatchComp) {
        if (!dispatchComp.method.exists()) {
            return;
        }

        if (dispatchComp.method.data(ctx)->isClass()) {
            return;
        }

        auto methodArgs = dispatchComp.method.data(ctx)->arguments();
        ENFORCE(!methodArgs.empty());
        SymbolRef bspec = methodArgs.back();
        ENFORCE(bspec.exists() && bspec.data(ctx)->isBlockArgument());
        e.addErrorSection(ErrorSection({
            ErrorLine::from(bspec.data(ctx)->loc(), "Method `{}` has specified `{}` as `{}`",
                            dispatchComp.method.data(ctx)->show(ctx), bspec.data(ctx)->argumentName(ctx),
                            blockType->show(ctx)),
        }));
    }

    static TypePtr simulateCall(Context ctx, const TypeAndOrigins *receiver, DispatchArgs innerArgs,
                                shared_ptr<SendAndBlockLink> link, TypePtr passedInBlockType, Loc callLoc,
                                Loc blockLoc) {
        auto dispatched = receiver->type->dispatchCall(ctx, innerArgs);
        for (auto &comp : dispatched.components) {
            for (auto &err : comp.errors) {
                ctx.state._error(std::move(err));
            }
        }
        // We use isSubTypeUnderConstraint here with a TypeConstraint, so that we discover the correct generic bounds
        // as we do the subtyping check.
        TypeConstraint *constr = link->constr.get();
        if (link->blockPreType &&
            !Types::isSubTypeUnderConstraint(ctx, *constr, passedInBlockType, link->blockPreType)) {
            ClassType *passedInProcClass = cast_type<ClassType>(passedInBlockType.get());
            auto nonNilableBlockType = Types::dropSubtypesOf(ctx, link->blockPreType, Symbols::NilClass());
            if (passedInProcClass && passedInProcClass->symbol == Symbols::Proc() &&
                Types::isSubType(ctx, nonNilableBlockType, passedInBlockType)) {
                // If a block of unknown arity is passed in, but the function was declared with a known arity,
                // raise an error in strict mode.
                // This could occur, for example, when using Method#to_proc, since we type it as returning a `Proc`.
                if (auto e = ctx.state.beginError(blockLoc, errors::Infer::ProcArityUnknown)) {
                    e.setHeader("Cannot use a `{}` with unknown arity as a `{}`", "Proc",
                                link->blockPreType->show(ctx));
                    if (dispatched.components.size() == 1) {
                        Magic_callWithBlock::showLocationOfArgDefn(ctx, e, link->blockPreType,
                                                                   dispatched.components[0]);
                    }
                }

                // Create a new proc of correct arity, with everything as untyped,
                // and then use this type instead of passedInBlockType in later subtype checks.
                // This allows the generic parameters to be instantiated with untyped rather than bottom.
                if (std::optional<int> procArity = Magic_callWithBlock::getArityForBlock(nonNilableBlockType)) {
                    vector<core::TypePtr> targs(*procArity + 1, core::Types::untypedUntracked());
                    auto procWithCorrectArity = core::Symbols::Proc(*procArity);
                    passedInBlockType = make_type<core::AppliedType>(procWithCorrectArity, targs);
                }
            } else if (auto e = ctx.state.beginError(blockLoc, errors::Infer::MethodArgumentMismatch)) {
                e.setHeader("`{}` doesn't match `{}` for block argument", passedInBlockType->show(ctx),
                            link->blockPreType->show(ctx));
                if (dispatched.components.size() == 1) {
                    Magic_callWithBlock::showLocationOfArgDefn(ctx, e, link->blockPreType, dispatched.components[0]);
                }
            }
        }

        for (auto &dispatchComp : dispatched.components) {
            if (!dispatchComp.method.exists()) {
                continue;
            }

            if (dispatchComp.method.data(ctx)->isClass()) {
                continue;
            }

            auto methodArgs = dispatchComp.method.data(ctx)->arguments();
            ENFORCE(!methodArgs.empty());
            SymbolRef bspec = methodArgs.back();
            ENFORCE(bspec.exists() && bspec.data(ctx)->isBlockArgument());

            auto bspecType = bspec.data(ctx)->resultType;
            if (bspecType) {
                // This subtype check is here to discover the correct generic bounds.
                Types::isSubTypeUnderConstraint(ctx, *constr, passedInBlockType, bspecType);
            }
        }

        if (!constr->solve(ctx)) {
            if (auto e = ctx.state.beginError(callLoc, errors::Infer::GenericMethodConstaintUnsolved)) {
                e.setHeader("Could not find valid instantiation of type parameters");
            }
            return core::Types::untypedUntracked();
        }

        if (!constr->isEmpty() && constr->isSolved()) {
            dispatched.returnType = Types::instantiate(ctx, dispatched.returnType, *(constr));
        }
        return dispatched.returnType;
    }

public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] is the block
        // args[3...] are the remaining arguements
        // equivalent to (args[0]).args[1](*args[3..], &args[2])

        if (args.args.size() < 3) {
            return core::Types::untypedUntracked();
        }
        auto &receiver = args.args[0];
        if (receiver->type->isUntyped()) {
            return receiver->type;
        }

        if (!receiver->type->isFullyDefined()) {
            return core::Types::untypedUntracked();
        }

        if (core::cast_type<core::TypeVar>(args.args[2]->type.get())) {
            if (auto e = ctx.state.beginError(args.locs.args[2], core::errors::Infer::GenericPassedAsBlock)) {
                e.setHeader("Passing generics as block arguments is not supported");
            }
            return Types::untypedUntracked();
        }

        auto *lit = cast_type<LiteralType>(args.args[1]->type.get());
        if (!lit || !lit->derivesFrom(ctx, Symbols::Symbol())) {
            return core::Types::untypedUntracked();
        }
        NameRef fn(ctx.state, (u4)lit->value);

        InlinedVector<TypeAndOrigins, 2> sendArgStore;
        InlinedVector<Loc, 2> sendArgLocs;
        for (int i = 3; i < args.args.size(); i++) {
            sendArgStore.emplace_back(*args.args[i]);
            sendArgLocs.emplace_back(args.locs.args[i]);
        }
        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        sendArgs.reserve(sendArgStore.size());
        for (auto &arg : sendArgStore) {
            sendArgs.emplace_back(&arg);
        }
        CallLocs sendLocs{args.locs.call, args.locs.args[0], sendArgLocs};

        TypePtr finalBlockType =
            Magic_callWithBlock::typeToProc(ctx, args.args[2]->type, args.locs.call, args.locs.args[2]);
        std::optional<int> blockArity = Magic_callWithBlock::getArityForBlock(finalBlockType);
        auto link = make_shared<core::SendAndBlockLink>(Symbols::noSymbol(), fn, blockArity);

        DispatchArgs innerArgs{fn, sendLocs, sendArgs, receiver->type, receiver->type, link};

        return Magic_callWithBlock::simulateCall(ctx, receiver, innerArgs, link, finalBlockType, args.locs.args[2],
                                                 args.locs.call);
    }
} Magic_callWithBlock;

class Magic_callWithSplatAndBlock : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] are the splat arguments
        // args[3] is the block

        if (args.args.size() != 4) {
            return core::Types::untypedUntracked();
        }
        auto &receiver = args.args[0];
        if (receiver->type->isUntyped()) {
            return receiver->type;
        }

        if (!receiver->type->isFullyDefined()) {
            return core::Types::untypedUntracked();
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

        if (core::cast_type<core::TypeVar>(args.args[3]->type.get())) {
            if (auto e = ctx.state.beginError(args.locs.args[3], core::errors::Infer::GenericPassedAsBlock)) {
                e.setHeader("Passing generics as block arguments is not supported");
            }
            return Types::untypedUntracked();
        }

        InlinedVector<TypeAndOrigins, 2> sendArgStore;
        InlinedVector<const TypeAndOrigins *, 2> sendArgs =
            Magic_callWithSplat::generateSendArgs(tuple, sendArgStore, args.locs.args[2]);
        InlinedVector<Loc, 2> sendArgLocs(tuple->elems.size(), args.locs.args[2]);
        CallLocs sendLocs{args.locs.call, args.locs.args[0], sendArgLocs};

        TypePtr finalBlockType =
            Magic_callWithBlock::typeToProc(ctx, args.args[3]->type, args.locs.call, args.locs.args[3]);
        std::optional<int> blockArity = Magic_callWithBlock::getArityForBlock(finalBlockType);
        auto link = make_shared<core::SendAndBlockLink>(Symbols::noSymbol(), fn, blockArity);

        DispatchArgs innerArgs{fn, sendLocs, sendArgs, receiver->type, receiver->type, link};

        return Magic_callWithBlock::simulateCall(ctx, receiver, innerArgs, link, finalBlockType, args.locs.args[3],
                                                 args.locs.call);
    }
} Magic_callWithSplatAndBlock;

class Tuple_squareBrackets : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
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
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
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
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
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
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
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
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        return args.selfType;
    }
} Tuple_to_a;

class Tuple_concat : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        vector<TypePtr> elems;
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
        return TupleType::build(ctx, std::move(elems));
    }
} Tuple_concat;

class Shape_merge : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
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
        for (auto &keyType : rhs->keys) {
            auto key = cast_type<LiteralType>(keyType.get());
            auto &value = rhs->values[&keyType - &rhs->keys.front()];
            auto fnd =
                absl::c_find_if(keys, [&key](auto &lit) { return key->equals(*cast_type<LiteralType>(lit.get())); });
            if (fnd == keys.end()) {
                keys.emplace_back(keyType);
                values.emplace_back(value);
            } else {
                values[fnd - keys.begin()] = value;
            }
        }

        return make_type<ShapeType>(Types::hashOfUntyped(), std::move(keys), std::move(values));
    }
} Shape_merge;

class Array_flatten : public IntrinsicMethod {
    // Flattens a (nested) array all way down to its (inner) element type, stopping if we hit the depth limit first.
    static TypePtr recursivelyFlattenArrays(Context ctx, const TypePtr &type, const int64_t depth) {
        ENFORCE(type != nullptr);

        if (depth == 0) {
            return type;
        }
        const int newDepth = depth - 1;

        TypePtr result;
        typecase(
            type.get(),

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

            [&](Type *t) { result = std::move(type); });
        return result;
    }

public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        // Unwrap the array one time to get the element type (we'll rewrap it down at the bottom)
        TypePtr element;
        if (auto *ap = cast_type<AppliedType>(thisType)) {
            ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(ctx)->derivesFrom(ctx, Symbols::Array()));
            ENFORCE(!ap->targs.empty());
            element = ap->targs.front();
        } else if (auto *tuple = cast_type<TupleType>(thisType)) {
            element = tuple->elementType();
        } else {
            ENFORCE(false, "Array#flatten on unexpected type: {}", args.selfType->show(ctx));
        }

        int64_t depth = INT64_MAX;
        if (args.args.size() == 1) {
            auto argTyp = args.args[0]->type;
            ENFORCE(args.locs.args.size() == 1, "Mismatch between args.size() and args.locs.args.size(): {}",
                    args.locs.args.size());
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
            ENFORCE(args.args.empty(), "Array#flatten passed too many args: {}", args.args.size());
        }

        return Types::arrayOf(ctx, recursivelyFlattenArrays(ctx, element, depth));
    }
} Array_flatten;

class Array_compact : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        TypePtr element;
        if (auto *ap = cast_type<AppliedType>(thisType)) {
            ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(ctx)->derivesFrom(ctx, Symbols::Array()));
            ENFORCE(!ap->targs.empty());
            element = ap->targs.front();
        } else if (auto *tuple = cast_type<TupleType>(thisType)) {
            element = tuple->elementType();
        } else {
            ENFORCE(false, "Array#compact on unexpected type: {}", args.selfType->show(ctx));
        }
        auto ret = Types::approximateSubtract(ctx, element, Types::nilClass());
        return Types::arrayOf(ctx, ret);
    }
} Array_compact;

class Kernel_proc : public IntrinsicMethod {
public:
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        if (args.block == nullptr) {
            return core::Types::untypedUntracked();
        }

        std::optional<int> numberOfPositionalBlockParams = args.block->numberOfPositionalBlockParams;
        if (!numberOfPositionalBlockParams || *numberOfPositionalBlockParams > core::Symbols::MAX_PROC_ARITY) {
            return core::Types::procClass();
        }
        vector<core::TypePtr> targs(*numberOfPositionalBlockParams + 1, core::Types::untypedUntracked());
        auto procClass = core::Symbols::Proc(*numberOfPositionalBlockParams);
        return make_type<core::AppliedType>(procClass, targs);
    }
} Kernel_proc;

class enumerable_to_h : public IntrinsicMethod {
public:
    // Forward Enumerable.to_h to RubyType.enumerable_to_h[self]
    TypePtr apply(Context ctx, DispatchArgs args, const Type *thisType) const override {
        auto hash = make_type<ClassType>(core::Symbols::Sorbet_Private_Static().data(ctx)->lookupSingletonClass(ctx));
        InlinedVector<Loc, 2> argLocs{args.locs.receiver};
        CallLocs locs{
            args.locs.call,
            args.locs.call,
            argLocs,
        };
        TypeAndOrigins myType{args.selfType, {args.locs.receiver}};
        InlinedVector<const TypeAndOrigins *, 2> innerArgs{&myType};

        DispatchArgs dispatch{
            core::Names::enumerable_to_h(), locs, innerArgs, hash, hash, nullptr,
        };
        auto dispatched = hash->dispatchCall(ctx, dispatch);
        for (auto &comp : dispatched.components) {
            for (auto &err : comp.errors) {
                ctx.state._error(std::move(err));
            }
        }
        return dispatched.returnType;
    }
} enumerable_to_h;

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

    {Symbols::MagicSingleton(), false, Names::buildHash(), &Magic_buildHash},
    {Symbols::MagicSingleton(), false, Names::buildArray(), &Magic_buildArray},
    {Symbols::MagicSingleton(), false, Names::expandSplat(), &Magic_expandSplat},
    {Symbols::MagicSingleton(), false, Names::callWithSplat(), &Magic_callWithSplat},
    {Symbols::MagicSingleton(), false, Names::callWithBlock(), &Magic_callWithBlock},
    {Symbols::MagicSingleton(), false, Names::callWithSplatAndBlock(), &Magic_callWithSplatAndBlock},

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

    {Symbols::Enumerable(), false, Names::to_h(), &enumerable_to_h},
};

} // namespace sorbet::core
