#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/common.h"
#include "common/sort/sort.h"
#include "common/typecase.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/TypeErrorDiagnostics.h"
#include "core/Types.h"
#include "core/errors/infer.h"
#include "core/errors/resolver.h"
#include <algorithm> // find_if, sort

#include "absl/strings/str_cat.h"

template class std::vector<sorbet::core::SymbolRef>;
using namespace std;

namespace sorbet::core {

namespace {

const bool IMPLICIT_CONVERSION_ALLOWS_PRIVATE = true;

DispatchResult dispatchCallProxyType(const GlobalState &gs, TypePtr und, const DispatchArgs &args) {
    categoryCounterInc("dispatch_call", "proxytype");
    return und.dispatchCall(gs, args.withThisRef(und));
}

bool allComponentsPresent(DispatchResult &res) {
    if (!res.main.method.exists()) {
        return false;
    }
    if (!res.secondary || res.secondaryKind == DispatchResult::Combinator::AND) {
        return true;
    }
    return allComponentsPresent(*res.secondary);
}
} // namespace

bool NamedLiteralType::derivesFrom(const GlobalState &gs, core::ClassOrModuleRef klass) const {
    return underlying(gs).derivesFrom(gs, klass);
}

bool IntegerLiteralType::derivesFrom(const GlobalState &gs, core::ClassOrModuleRef klass) const {
    return underlying(gs).derivesFrom(gs, klass);
}

bool FloatLiteralType::derivesFrom(const GlobalState &gs, core::ClassOrModuleRef klass) const {
    return underlying(gs).derivesFrom(gs, klass);
}

bool ShapeType::derivesFrom(const GlobalState &gs, core::ClassOrModuleRef klass) const {
    return underlying(gs).derivesFrom(gs, klass);
}

bool TupleType::derivesFrom(const GlobalState &gs, core::ClassOrModuleRef klass) const {
    return underlying(gs).derivesFrom(gs, klass);
}

DispatchResult NamedLiteralType::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    return dispatchCallProxyType(gs, underlying(gs), args);
}

DispatchResult IntegerLiteralType::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    return dispatchCallProxyType(gs, underlying(gs), args);
}

DispatchResult FloatLiteralType::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    return dispatchCallProxyType(gs, underlying(gs), args);
}

DispatchResult OrType::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    categoryCounterInc("dispatch_call", "ortype");
    auto leftRet = left.dispatchCall(gs, args.withSelfAndThisRef(left));
    auto rightRet = right.dispatchCall(gs, args.withSelfAndThisRef(right));
    return DispatchResult::merge(gs, DispatchResult::Combinator::OR, std::move(leftRet), std::move(rightRet));
}

TypePtr OrType::getCallArguments(const GlobalState &gs, NameRef name) const {
    auto largs = left.getCallArguments(gs, name);
    auto rargs = right.getCallArguments(gs, name);
    if (!largs) {
        largs = Types::untypedUntracked();
    }
    if (!rargs) {
        rargs = Types::untypedUntracked();
    }
    return Types::glb(gs, largs, rargs);
}

DispatchResult AndType::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    categoryCounterInc("dispatch_call", "andtype");
    // Tell dispatchCall to not produce any dispatch-related errors. They are very expensive to produce.
    auto leftRet = left.dispatchCall(gs, args.withThisRef(left).withErrorsSuppressed());
    auto rightRet = right.dispatchCall(gs, args.withThisRef(right).withErrorsSuppressed());

    // If either side is missing the method, dispatch to the other.
    auto leftOk = allComponentsPresent(leftRet);
    auto rightOk = allComponentsPresent(rightRet);
    if (leftOk && !rightOk) {
        return leftRet;
    }
    if (rightOk && !leftOk) {
        return rightRet;
    }
    if (!rightOk && !leftOk) {
        // Expensive case. Re-dispatch the calls with errors enabled so we can give the user an error.
        leftRet = left.dispatchCall(gs, args.withThisRef(left));
        rightRet = right.dispatchCall(gs, args.withThisRef(right));
    }

    auto resultType = Types::all(gs, leftRet.returnType, rightRet.returnType);
    return DispatchResult::merge(gs, DispatchResult::Combinator::AND, std::move(leftRet), std::move(rightRet));
}

TypePtr AndType::getCallArguments(const GlobalState &gs, NameRef name) const {
    auto l = left.getCallArguments(gs, name);
    auto r = right.getCallArguments(gs, name);
    if (l == nullptr) {
        return r;
    }
    if (r == nullptr) {
        return l;
    }
    return Types::any(gs, l, r);
}

DispatchResult ShapeType::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    categoryCounterInc("dispatch_call", "shapetype");
    auto method = Symbols::Shape().data(gs)->findMethod(gs, args.name);
    if (method.exists()) {
        auto *intrinsic = method.data(gs)->getIntrinsic();
        if (intrinsic != nullptr) {
            DispatchComponent comp{args.selfType, method, {}, nullptr, nullptr, nullptr, ArgInfo{}, nullptr};
            DispatchResult res{nullptr, std::move(comp)};
            intrinsic->apply(gs, args, res);
            if (res.returnType != nullptr) {
                return res;
            }
        }
    }
    return dispatchCallProxyType(gs, underlying(gs), args);
}

DispatchResult TupleType::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    categoryCounterInc("dispatch_call", "tupletype");
    auto method = Symbols::Tuple().data(gs)->findMethod(gs, args.name);
    if (method.exists()) {
        auto *intrinsic = method.data(gs)->getIntrinsic();
        if (intrinsic != nullptr) {
            DispatchComponent comp{args.selfType, method, {}, nullptr, nullptr, nullptr, ArgInfo{}, nullptr};
            DispatchResult res{nullptr, std::move(comp)};
            intrinsic->apply(gs, args, res);
            if (res.returnType != nullptr) {
                return res;
            }
        }
    }
    return dispatchCallProxyType(gs, underlying(gs), args);
}

namespace {
void autocorrectReceiver(const core::GlobalState &gs, core::ErrorBuilder &e, core::Loc receiverLoc,
                         core::NameRef methodName) {
    if (receiverLoc.exists() && (gs.suggestUnsafe.has_value())) {
        auto wrapInFn = gs.suggestUnsafe.value();
        if (receiverLoc.empty()) {
            auto shortName = methodName.shortName(gs);
            auto beginAdjust = -2;                     // (&
            auto endAdjust = 1 + shortName.size() + 1; // :foo)
            auto blockPassLoc = receiverLoc.adjust(gs, beginAdjust, endAdjust);
            if (blockPassLoc.exists()) {
                auto blockPassSource = blockPassLoc.source(gs).value();
                if (blockPassSource == fmt::format("(&:{})", shortName)) {
                    e.replaceWith(fmt::format("Expand to block with `{}`", wrapInFn), blockPassLoc,
                                  " {{ |x| {}(x).{} }}", wrapInFn, shortName);
                }
            }
        } else {
            e.replaceWith(fmt::format("Wrap in `{}`", wrapInFn), receiverLoc, "{}({})", wrapInFn,
                          receiverLoc.source(gs).value());
        }
    }
}

void addUnconstrainedIsaGenericNote(const GlobalState &gs, ErrorBuilder &e, SymbolRef definition, NameRef methodName,
                                    string_view genericKind) {
    if (methodName == Names::isA_p()) {
        e.addErrorNote("Use `{}` instead of `{}` to check the type of an unconstrained generic type {}", "case",
                       methodName.show(gs), genericKind);
    } else if (methodName == Names::nil_p()) {
        e.addErrorNote("Use `{}` instead of `{}` to check whether an unconstrained generic type {} is nil",
                       "case ... when nil", methodName.show(gs), genericKind);
    } else if (definition.isTypeMember()) {
        e.addErrorSection(ErrorSection(
            ErrorColors::format("Consider adding an `{}` bound to `{}` here", "upper", definition.show(gs)),
            {{definition.loc(gs), ""}}));

    } else {
        auto showOptions = ShowOptions().withUseValidSyntax();
        auto wrapped = fmt::format("T.all({}, Constraint)", definition.show(gs, showOptions));
        e.addErrorNote("Consider using `{}` to place a constraint on this type", wrapped);
    }
}
} // namespace

DispatchResult SelfTypeParam::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    auto emptyResult = DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noMethod());
    if (this->definition.isTypeArgument()) {
        if (args.suppressErrors) {
            // Short circuit here to avoid constructing an expensive error message.
            return emptyResult;
        }
        auto e = gs.beginError(args.errLoc(), errors::Infer::CallOnTypeArgument);
        if (e) {
            auto thisStr = args.thisType.show(gs);
            if (args.fullType.type != args.thisType) {
                e.setHeader("Call to method `{}` on generic type `{}` component of `{}`", args.name.show(gs), thisStr,
                            args.fullType.type.show(gs));
            } else {
                e.setHeader("Call to method `{}` on unconstrained generic type `{}`", args.name.show(gs), thisStr);
            }
            e.addErrorSection(args.fullType.explainGot(gs, args.originForUninitialized));
            autocorrectReceiver(gs, e, args.receiverLoc(), args.name);
            addUnconstrainedIsaGenericNote(gs, e, this->definition, args.name, "parameter");
        }
        emptyResult.main.errors.emplace_back(e.build());
        return emptyResult;
    } else {
        ENFORCE(this->definition.isTypeMember());
        auto typeMember = this->definition.asTypeMemberRef();
        auto &lambdaParam = cast_type_nonnull<LambdaParam>(typeMember.data(gs)->resultType);
        auto upperBound = lambdaParam.upperBound;
        if (upperBound.isTop()) {
            if (args.suppressErrors) {
                return emptyResult;
            }

            auto e = gs.beginError(args.errLoc(), errors::Infer::CallOnUnboundedTypeMember);
            if (e) {
                auto member = typeMember.data(gs)->owner.asClassOrModuleRef().data(gs)->attachedClass(gs).exists()
                                  ? "template"
                                  : "member";
                auto thisStr = args.thisType.show(gs);
                if (args.fullType.type != args.thisType) {
                    e.setHeader("Call to method `{}` on unbounded type {} `{}` component of `{}`", args.name.show(gs),
                                member, thisStr, args.fullType.type.show(gs));
                } else {
                    e.setHeader("Call to method `{}` on unbounded type {} `{}`", args.name.show(gs), member, thisStr);
                }
                e.addErrorSection(args.fullType.explainGot(gs, args.originForUninitialized));
                autocorrectReceiver(gs, e, args.receiverLoc(), args.name);
                addUnconstrainedIsaGenericNote(gs, e, this->definition, args.name, member);
            }
            emptyResult.main.errors.emplace_back(e.build());
            return emptyResult;
        }

        return upperBound.dispatchCall(gs, args.withThisRef(upperBound));
    }
}

namespace {

unique_ptr<Error> matchArgType(const GlobalState &gs, TypeConstraint &constr, Loc receiverLoc, ClassOrModuleRef inClass,
                               MethodRef method, const TypeAndOrigins &argTpe, const ArgInfo &argSym,
                               const TypePtr &selfType, const vector<TypePtr> &targs, Loc argLoc,
                               Loc originForUninitialized, bool mayBeSetter = false) {
    TypePtr expectedType = Types::resultTypeAsSeenFrom(gs, argSym.type, method.data(gs)->owner, inClass, targs);
    if (!expectedType) {
        expectedType = Types::untyped(method);
    }

    expectedType = Types::replaceSelfType(gs, expectedType, selfType);

    if (Types::isSubTypeUnderConstraint(gs, constr, argTpe.type, expectedType, UntypedMode::AlwaysCompatible)) {
        if (expectedType.isUntyped()) {
            // TODO(jez) We should have code like this, but currently there are too many places that
            // are fine "accepting anything" that are typed as `T.untyped`.
            //
            // We should take a pass at fixing a lot of those in our RBI files, and then circle back
            // to enabling this error.
            //
            // auto what = core::errors::Infer::errorClassForUntyped(gs, argLoc.file());
            // if (auto e = gs.beginError(argLoc, what)) {
            //     e.setHeader("Method parameter `{}` is declared with `{}`", argSym.argumentName(gs), "T.untyped");
            //     TypeErrorDiagnostics::explainUntyped(gs, e, what, expectedType, argSym.loc, originForUninitialized);
            //     return e.build();
            // }
        } else if (argTpe.type.isUntyped() && expectedType != Types::top()) {
            auto what = core::errors::Infer::errorClassForUntyped(gs, argLoc.file(), argTpe.type);
            if (auto e = gs.beginError(argLoc, what)) {
                e.setHeader("Argument passed to parameter `{}` is `{}`", argSym.argumentName(gs), "T.untyped");
                auto for_ =
                    ErrorColors::format("argument `{}` of method `{}`", argSym.argumentName(gs), method.show(gs));
                e.addErrorSection(TypeAndOrigins::explainExpected(gs, expectedType, argSym.loc, for_));
                TypeErrorDiagnostics::explainUntyped(gs, e, what, argTpe, originForUninitialized);
                return e.build();
            }
        }
        return nullptr;
    }

    if (auto e = gs.beginError(argLoc, errors::Infer::MethodArgumentMismatch)) {
        if (mayBeSetter && method.data(gs)->name.isSetter(gs)) {
            e.setHeader("Assigning a value to `{}` that does not match expected type `{}`", argSym.argumentName(gs),
                        expectedType.show(gs));
        } else {
            e.setHeader("Expected `{}` but found `{}` for argument `{}`", expectedType.show(gs), argTpe.type.show(gs),
                        argSym.argumentName(gs));
            auto for_ = ErrorColors::format("argument `{}` of method `{}`", argSym.argumentName(gs), method.show(gs));
            e.addErrorSection(TypeAndOrigins::explainExpected(gs, expectedType, argSym.loc, for_));
        }
        e.addErrorSection(argTpe.explainGot(gs, originForUninitialized));
        core::TypeErrorDiagnostics::explainTypeMismatch(gs, e, expectedType, argTpe.type);
        TypeErrorDiagnostics::maybeAutocorrect(gs, e, argLoc, constr, expectedType, argTpe.type);
        return e.build();
    }
    return nullptr;
}

unique_ptr<Error> missingArg(const GlobalState &gs, Loc argsLoc, Loc receiverLoc, MethodRef method, const ArgInfo &arg,
                             ClassOrModuleRef inClass, const vector<TypePtr> &targs) {
    if (auto e = gs.beginError(argsLoc, errors::Infer::MethodArgumentCountMismatch)) {
        auto argName = arg.name.show(gs);
        e.setHeader("Missing required keyword argument `{}` for method `{}`", argName, method.show(gs));
        auto expectedType = Types::resultTypeAsSeenFrom(gs, arg.type, method.data(gs)->owner, inClass, targs);
        if (expectedType == nullptr) {
            expectedType = Types::untyped(method);
        }
        e.addErrorLine(arg.loc, "Keyword argument `{}` declared to expect type `{}` here:", argName,
                       expectedType.show(gs));
        return e.build();
    }
    return nullptr;
}

size_t getArity(const GlobalState &gs, MethodRef method) {
    ENFORCE(!method.data(gs)->arguments.empty(), "Every method should have at least a block arg.");
    ENFORCE(method.data(gs)->arguments.back().flags.isBlock, "Last arg should be the block arg.");

    const auto &arguments = method.data(gs)->arguments;
    if (absl::c_any_of(arguments, [&](const auto &arg) { return arg.flags.isRepeated && !arg.flags.isKeyword; })) {
        return SIZE_MAX;
    };

    // Don't count the block arg in the arity
    return method.data(gs)->arguments.size() - 1;
}

struct GuessOverloadCandidate {
    MethodRef candidate;
    shared_ptr<TypeConstraint> constr;
};

// Guess overload. The way we guess is only arity based - we will return the overload that has the smallest number of
// arguments that is >= args.size()
MethodRef guessOverload(const GlobalState &gs, ClassOrModuleRef inClass, MethodRef primary, uint16_t numPosArgs,
                        InlinedVector<const TypeAndOrigins *, 2> &args, const vector<TypePtr> &targs,
                        const shared_ptr<const SendAndBlockLink> &block) {
    counterInc("calls.overloaded_invocations");
    MethodRef fallback = primary;
    vector<MethodRef> allCandidates;

    allCandidates.emplace_back(primary);
    { // create candidates and sort them by number of arguments(stable by symbol id)
        size_t i = 0;
        MethodRef current = primary;
        while (current.data(gs)->flags.isOverloaded) {
            i++;
            NameRef overloadName = gs.lookupNameUnique(UniqueNameKind::Overload, primary.data(gs)->name, i);
            MethodRef overload = primary.data(gs)->owner.data(gs)->findMethod(gs, overloadName);
            if (!overload.exists()) {
                Exception::raise("Corruption of overloads?");
            } else {
                allCandidates.emplace_back(overload);
                current = overload;
            }
        }

        fast_sort(allCandidates, [&](const auto &s1, const auto &s2) -> bool {
            auto s1Arity = getArity(gs, s1);
            auto s2Arity = getArity(gs, s2);
            if (s1Arity < s2Arity) {
                return true;
            }
            if (s1Arity == s2Arity) {
                return s1.id() < s2.id();
            }
            return false;
        });
    }

    vector<GuessOverloadCandidate> allCandidatesWithConstraints;

    for (const auto &candidate : allCandidates) {
        if (!candidate.data(gs)->flags.isGenericMethod) {
            allCandidatesWithConstraints.emplace_back(GuessOverloadCandidate{candidate, nullptr});
            continue;
        }

        // Make a new TypeConstraint with everything in the domain solving to `T.untyped`.
        //
        // This allows Sorbet to consider or reject an overload with a parameter like
        // `T::Array[T.type_parameter(:U)]` based on whether the argument is `String`--in that case,
        // it doesn't matter what the `T.type_parameter(:U)` is, because `String` is not an `Array`.
        auto constr = make_unique<TypeConstraint>();
        for (auto typeArgument : candidate.data(gs)->typeArguments()) {
            constr->rememberIsSubtype(gs, typeArgument.data(gs)->resultType, Types::untypedUntracked());
        }
        if (!constr->solve(gs)) {
            Exception::raise("Constraint should always solve after creating TypeConstraint with only untyped bounds");
        }

        allCandidatesWithConstraints.emplace_back(GuessOverloadCandidate{candidate, move(constr)});
    }

    // Copy the vector
    auto leftCandidates = allCandidatesWithConstraints;

    {
        auto checkArg = [&](auto i, const TypePtr &arg) {
            for (auto it = leftCandidates.begin(); it != leftCandidates.end(); /* nothing*/) {
                const auto &[candidate, constr] = *it;
                const auto &arguments = candidate.data(gs)->arguments;
                TypePtr argTypeRaw;
                auto arity = getArity(gs, candidate);
                if (i < arguments.size() - 1) {
                    argTypeRaw = arguments[i].type;
                } else if (arity == SIZE_MAX) {
                    auto restArg = absl::c_find_if(
                        arguments, [&](const auto &arg) { return arg.flags.isRepeated && !arg.flags.isKeyword; });
                    ENFORCE(restArg != arguments.end())
                    argTypeRaw = restArg->type;
                } else {
                    it = leftCandidates.erase(it);
                    continue;
                }

                auto argType = Types::resultTypeAsSeenFrom(gs, argTypeRaw, candidate.data(gs)->owner, inClass, targs);
                if (constr == nullptr) {
                    if (!Types::isSubType(gs, arg, argType)) {
                        it = leftCandidates.erase(it);
                        continue;
                    }
                } else {
                    if (!Types::isSubTypeUnderConstraint(gs, *constr, arg, argType, UntypedMode::AlwaysCompatible)) {
                        it = leftCandidates.erase(it);
                        continue;
                    }
                }
                ++it;
            }
        };

        // Lets see if we can filter them out using arguments.
        for (auto i = 0; i < numPosArgs; ++i) {
            checkArg(i, args[i]->type);
        }

        // If keyword args are present, interpret them as an untyped hash
        if (numPosArgs < args.size()) {
            checkArg(numPosArgs, Types::hashOfUntyped());
        }
    }
    if (leftCandidates.empty()) {
        leftCandidates = allCandidatesWithConstraints;
    } else {
        fallback = leftCandidates[0].candidate;
    }

    { // keep only candidates that have a block iff we are passing one
        for (auto it = leftCandidates.begin(); it != leftCandidates.end(); /* nothing*/) {
            const auto &[candidate, constr] = *it;
            const auto &args = candidate.data(gs)->arguments;
            ENFORCE(!args.empty(), "Should at least have a block argument.");
            const auto &lastArg = args.back();
            auto mentionsBlockArg = !lastArg.isSyntheticBlockArgument();
            if (block != nullptr) {
                if (!mentionsBlockArg || lastArg.type == Types::nilClass()) {
                    it = leftCandidates.erase(it);
                    continue;
                }
                if (auto blockParamType = cast_type<AppliedType>(lastArg.type)) {
                    if (auto procArity = Types::getProcArity(*blockParamType)) {
                        if (auto fixedArity = block->fixedArity()) {
                            if (procArity != block->fixedArity()) {
                                it = leftCandidates.erase(it);
                                continue;
                            }
                        }
                    }
                }
            } else {
                if (mentionsBlockArg && lastArg.type != nullptr &&
                    (!lastArg.type.isFullyDefined() || !Types::isSubType(gs, Types::nilClass(), lastArg.type))) {
                    it = leftCandidates.erase(it);
                    continue;
                }
            }
            ++it;
        }
    }

    { // keep only candidates with closest arity
        struct Comp {
            const GlobalState &gs;

            bool operator()(GuessOverloadCandidate &s, int i) const {
                return getArity(gs, s.candidate) < i;
            }

            bool operator()(int i, GuessOverloadCandidate &s) const {
                return i < getArity(gs, s.candidate);
            }

            Comp(const GlobalState &gs) : gs(gs){};
        } cmp(gs);

        auto er = absl::c_equal_range(leftCandidates, args.size(), cmp);
        if (er.first != leftCandidates.end()) {
            leftCandidates.erase(leftCandidates.begin(), er.first);
        }
    }

    if (!leftCandidates.empty()) {
        return leftCandidates[0].candidate;
    }
    return fallback;
}

struct ArityComponents {
    int required;
    int optional;
    bool repeated;
};

ArityComponents arityComponents(const GlobalState &gs, MethodRef method) {
    int required = 0, optional = 0;
    bool repeated = false;
    for (const auto &arg : method.data(gs)->arguments) {
        if (arg.flags.isKeyword || arg.flags.isBlock) {
            // ignore
        } else if (arg.flags.isDefault) {
            ++optional;
        } else if (arg.flags.isRepeated) {
            repeated = true;
        } else {
            ++required;
        }
    }
    return {required, optional, repeated};
}

string prettyArity(const GlobalState &gs, MethodRef method) {
    auto [required, optional, repeated] = arityComponents(gs, method);
    if (repeated) {
        return absl::StrCat(required, "+");
    } else if (optional > 0) {
        return absl::StrCat(required, "..", required + optional);
    } else {
        return to_string(required);
    }
}

void maybeSuggestUnsafeKwsplat(const core::GlobalState &gs, core::ErrorBuilder &e, core::Loc kwSplatArgLoc) {
    if (!kwSplatArgLoc.exists() || kwSplatArgLoc.empty()) {
        return;
    }

    auto replaceLoc = kwSplatArgLoc;
    string maybeStarStar = "**";
    if (kwSplatArgLoc.adjustLen(gs, 0, 2).source(gs) == "**") {
        replaceLoc = kwSplatArgLoc.adjust(gs, 2, 0);
        maybeStarStar = "";
    }

    auto suggestUnsafe = gs.suggestUnsafe.value_or("T.unsafe"s);
    auto title = fmt::format("Wrap in `{}`", suggestUnsafe);
    auto replaceValue = replaceLoc.source(gs).value();
    if (absl::c_all_of(replaceValue, [](char c) { return absl::ascii_isalnum(c) || c == '_'; }) ||
        (absl::StartsWith(replaceValue, "{") && absl::EndsWith(replaceValue, "}"))) {
        e.replaceWith(title, replaceLoc, "{}{}({})", maybeStarStar, suggestUnsafe, replaceValue);
    } else {
        // wraps inside of T.unsafe(...) in `{...}`
        auto extraStarStar = replaceLoc != kwSplatArgLoc ? "**" : "";
        e.replaceWith(title, replaceLoc, "{}{}({{{}{}}})", maybeStarStar, suggestUnsafe, extraStarStar, replaceValue);
    }
}

// Ensure that a ShapeType used as a keyword args splat in a send has only symbol keys present.
const ShapeType *fromKwargsHash(const GlobalState &gs, const TypePtr &ty) {
    auto *hash = cast_type<ShapeType>(ty);
    if (hash == nullptr) {
        return nullptr;
    }

    if (!absl::c_all_of(hash->keys, [&gs](const auto &key) {
            if (!isa_type<NamedLiteralType>(key)) {
                return false;
            }

            auto klass = cast_type_nonnull<NamedLiteralType>(key).underlying(gs);
            if (!isa_type<ClassType>(klass)) {
                return false;
            }

            return cast_type_nonnull<ClassType>(klass).symbol == Symbols::Symbol();
        })) {
        return nullptr;
    }

    return hash;
}

// This implements Ruby's argument matching logic (assigning values passed to a
// method call to formal parameters of the method).
//
// Known incompleteness or inconsistencies with Ruby:
//  - Missing coercion to keyword arguments via `#to_hash`
//  - We never allow a non-shaped Hash to satisfy keyword arguments;
//    We should, at a minimum, probably allow one to satisfy an **kwargs : untyped
//    (with a subtype check on the key type, once we have generics)
DispatchResult dispatchCallSymbol(const GlobalState &gs, const DispatchArgs &args, core::ClassOrModuleRef symbol,
                                  const vector<TypePtr> &targs) {
    auto errLoc = args.errLoc();
    if (symbol == core::Symbols::untyped()) {
        auto what = core::errors::Infer::errorClassForUntyped(gs, args.locs.file, args.thisType);
        if (auto e = gs.beginError(errLoc, what)) {
            e.setHeader("Call to method `{}` on `{}`", args.name.show(gs), "T.untyped");
            TypeErrorDiagnostics::explainUntyped(gs, e, what, args.fullType, args.originForUninitialized);
        }

        return DispatchResult(Types::untyped(args.thisType.untypedBlame()), std::move(args.selfType),
                              Symbols::noMethod());
    } else if (symbol == Symbols::void_()) {
        if (!args.suppressErrors) {
            if (auto e = gs.beginError(errLoc, errors::Infer::UnknownMethod)) {
                e.setHeader("Cannot call method `{}` on void type", args.name.show(gs));
            }
        }
        return DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noMethod());
    } else if (symbol == Symbols::DeclBuilderForProcsSingleton() && args.name == Names::new_()) {
        if (!args.suppressErrors) {
            if (auto e = gs.beginError(errLoc, errors::Infer::MetaTypeDispatchCall)) {
                e.setHeader("Call to method `{}` on `{}` mistakes a type for a value", Names::new_().show(gs),
                            symbol.show(gs));
            }
        }
        return DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noMethod());
    } else if (args.name == Names::methodNameMissing()) {
        // We already reported a parse error earlier in the pipeline
        return DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noMethod());
    } else if (args.name == Names::untypedSuper()) {
        // Currently, `super` is only checked for the following cases:
        // - The receiver is a class
        // - The super call is not inside a block
        // No point in paying a full findMethodTransitive just to discover that.
        return DispatchResult(Types::untyped(Symbols::Magic_UntypedSource_super()), std::move(args.selfType),
                              Symbols::noMethod());
    }

    auto targetName = args.name;
    MethodRef mayBeOverloaded;
    if (targetName == Names::super()) {
        targetName = args.enclosingMethodForSuper;
        mayBeOverloaded = symbol.data(gs)->findParentMethodTransitive(gs, targetName);
    } else if (symbol != core::Symbols::top()) {
        // TODO(jez) It would be nice to make `core::Symbols::top()` not have `Object` as its ancestor,
        // in which case we could simply let the findMethodTransitive run and fail to find any methods
        mayBeOverloaded = symbol.data(gs)->findMethodTransitive(gs, targetName);
    }

    if (!mayBeOverloaded.exists() && gs.requiresAncestorEnabled) {
        // Before raising any error, we look if the method exists in all required ancestors by this symbol
        auto ancestors = symbol.data(gs)->requiredAncestorsTransitive(gs);
        for (auto ancst : ancestors) {
            mayBeOverloaded = ancst.symbol.data(gs)->findMethodTransitive(gs, targetName);
            if (mayBeOverloaded.exists()) {
                break;
            }
        }
    }

    if (!mayBeOverloaded.exists()) {
        auto result = DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noMethod());
        if (args.suppressErrors) {
            // Short circuit here to avoid constructing an expensive error message.
            return result;
        }

        auto unknownMethodCode = args.name == Names::super()
                                     // So we can attach super-specific autocorrects at some point in the future
                                     ? errors::Infer::UnknownSuperMethod
                                     : errors::Infer::UnknownMethod;

        // This is a hack. We want to always be able to build the error object
        // so that it is not immediately sent to GlobalState::_error
        // and recorded.
        // Instead, the error always should get queued up in the
        // errors list of the result so that the caller can deal with the error.
        auto e = gs.beginError(errLoc, unknownMethodCode);
        if (e) {
            string thisStr = args.thisType.show(gs);
            auto ancestorsOf = args.name == Names::super() ? "ancestors of " : "";
            auto isSetter = targetName.isSetter(gs);
            auto methodPrefix = isSetter ? "Setter method" : "Method";
            if (args.fullType.type != args.thisType) {
                e.setHeader("{} `{}` does not exist on {}`{}` component of `{}`", methodPrefix, targetName.show(gs),
                            ancestorsOf, thisStr, args.fullType.type.show(gs));
            } else {
                e.setHeader("{} `{}` does not exist on {}`{}`", methodPrefix, targetName.show(gs), ancestorsOf,
                            thisStr);
            }
            e.addErrorSection(args.fullType.explainGot(gs, args.originForUninitialized));

            // catch the special case of `interface!`, `abstract!`, `final!`, or `sealed!` and
            // suggest adding `extend T::Helpers`.
            if (args.name == core::Names::declareInterface() || args.name == core::Names::declareAbstract() ||
                args.name == core::Names::declareFinal() || args.name == core::Names::declareSealed() ||
                args.name == core::Names::mixesInClassMethods() ||
                (args.name == core::Names::requiresAncestor() && gs.requiresAncestorEnabled)) {
                auto attachedClass = symbol.data(gs)->attachedClass(gs);
                TypeErrorDiagnostics::maybeInsertDSLMethod(gs, e, args.locs.file, args.callLoc(), attachedClass,
                                                           Symbols::T_Helpers(), "");
            } else if (args.name == core::Names::sig()) {
                auto attachedClass = symbol.data(gs)->attachedClass(gs);
                TypeErrorDiagnostics::maybeInsertDSLMethod(gs, e, args.locs.file, args.callLoc(), attachedClass,
                                                           Symbols::T_Sig(), "");
            } else if (args.name == Names::super()) {
                // TODO(jez) Special error for super.
                // - If identical name exists as self/instance method in parent but we're currently
                //   an instance/self method, suggest changing enclosing method definition.
                // - If similar name exists in parent suggest renaming enclosing method definition
                //   (intialize -> initialize, etc.)
                // - Otherwise, suggest `T.bind`
                // Note: super is not a method call, and so all the logic in the case below to
                // compute autocorrects to potentially fix up the method call's receiver can't apply.
                e.addErrorNote("For help fixing `{}` errors: {}", "super", "https://sorbet.org/docs/typed-super");
            } else if (args.receiverLoc().exists() &&
                       (gs.suggestUnsafe.has_value() ||
                        (args.fullType.type != args.thisType && symbol == Symbols::NilClass()))) {
                auto wrapInFn = gs.suggestUnsafe.value_or("T.must");
                if (args.receiverLoc().empty()) {
                    auto shortName = args.name.shortName(gs);
                    auto beginAdjust = -2;                     // (&
                    auto endAdjust = 1 + shortName.size() + 1; // :foo)
                    auto blockPassLoc = args.receiverLoc().adjust(gs, beginAdjust, endAdjust);
                    if (blockPassLoc.exists()) {
                        auto blockPassSource = blockPassLoc.source(gs).value();
                        if (blockPassSource == fmt::format("(&:{})", shortName)) {
                            e.replaceWith(fmt::format("Expand to block with `{}`", wrapInFn), blockPassLoc,
                                          " {{ |x| {}(x).{} }}", wrapInFn, shortName);
                        }
                    }
                } else if (errLoc == args.funLoc() &&
                           args.funLoc().source(gs) == absl::StrCat(args.name.shortName(gs), "=")) {
                    e.replaceWith(fmt::format("Wrap in `{}`", wrapInFn), args.funLoc(), "= {}({}) {}", wrapInFn,
                                  args.receiverLoc().source(gs).value(), args.name.shortName(gs));
                } else {
                    e.replaceWith(fmt::format("Wrap in `{}`", wrapInFn), args.receiverLoc(), "{}({})", wrapInFn,
                                  args.receiverLoc().source(gs).value());
                }
            } else {
                auto canSkipFuzzyMatch = false;
                if (symbol.data(gs)->isModule()) {
                    auto objMeth = core::Symbols::Object().data(gs)->findMethodTransitive(gs, args.name);
                    if (objMeth.exists() && objMeth.data(gs)->owner.data(gs)->isModule()) {
                        auto objData = objMeth.data(gs);
                        auto ownerName = objData->owner.data(gs)->name.show(gs);
                        e.addErrorNote("`{}` is actually defined as a method on `{}`. To call it, either\n"
                                       "    `{}` in this module to ensure the method is always there, or\n"
                                       "    call the method using `{}` instead.",
                                       args.name.show(gs), ownerName, fmt::format("include {}", ownerName),
                                       fmt::format("{}.{}", ownerName, args.name.show(gs)));
                        if (args.receiverLoc().exists() && args.receiverLoc().empty()) {
                            e.replaceWith("Prefix with `Kernel.`", args.receiverLoc(), "Kernel.");
                        }
                    }
                } else if (!symbol.data(gs)->attachedClass(gs).exists() &&
                           symbol.data(gs)->lookupSingletonClass(gs).exists()) {
                    auto singleton = symbol.data(gs)->lookupSingletonClass(gs);
                    auto methodOnSingleton = singleton.data(gs)->findMethodTransitive(gs, args.name);
                    if (methodOnSingleton.exists()) {
                        // isPrivateOk implies that the singleton class of the receiver is also the
                        // singleton class of the enclosing method, because the receiver is not a
                        // module and is syntactically `self`.
                        auto eitherLine =
                            args.isPrivateOk
                                ? ErrorLine::fromWithoutLoc(
                                      "Either:\n"
                                      "    - use `{}` to call it,\n"
                                      "    - remove `{}` from its definition to make it an instance method, or\n"
                                      "    - define the current method as a singleton class method using `{}`",
                                      ".class", "self.", "def self.")
                                : ErrorLine::fromWithoutLoc(
                                      "Either:\n"
                                      "    - use `{}` to call it, or\n"
                                      "    - remove `{}` from its definition to make it an instance method",
                                      ".class", "self.");

                        e.addErrorSection(
                            ErrorSection("There is a singleton class method with the same name:",
                                         {
                                             ErrorLine::from(methodOnSingleton.data(gs)->loc(), "Defined here"),
                                             move(eitherLine),
                                         }));

                        if (args.receiverLoc().exists() && args.receiverLoc().empty()) {
                            e.replaceWith("Insert `self.class.`", args.funLoc().copyWithZeroLength(), "self.class.");
                        } else if (args.receiverLoc().exists()) {
                            e.replaceWith("Insert `.class`", args.receiverLoc().copyEndWithZeroLength(), ".class");
                        }
                        canSkipFuzzyMatch = true;
                    }
                }

                if (!canSkipFuzzyMatch) {
                    auto alternatives = symbol.data(gs)->findMemberFuzzyMatch(gs, targetName);
                    for (auto alternative : alternatives) {
                        auto possibleSymbol = alternative.symbol;
                        if (!possibleSymbol.isClassOrModule() && !possibleSymbol.isMethod()) {
                            continue;
                        }

                        if (possibleSymbol.isMethod()) {
                            auto possibleMethod = possibleSymbol.asMethodRef().data(gs);
                            if ((possibleMethod->flags.isPrivate || possibleMethod->owner == Symbols::Kernel()) &&
                                !args.isPrivateOk) {
                                // Special-case Kernel methods, which should be treated as private, but aren't due to
                                // this bug: https://github.com/sorbet/sorbet/issues/4434
                                // (If we fix that bug, we can delete the `Kernel` reference above.)
                                continue;
                            }
                        }

                        if (isSetter && possibleSymbol.name(gs).lookupWithEq(gs) == args.name) {
                            e.addErrorLine(possibleSymbol.loc(gs),
                                           "Method `{}` defined here without a corresponding setter",
                                           possibleSymbol.name(gs).show(gs));
                            continue;
                        }

                        const auto toReplace = args.name.toString(gs);
                        if (args.funLoc().source(gs) != toReplace) {
                            auto suggestedName = possibleSymbol.isClassOrModule() ? possibleSymbol.show(gs) + ".new"
                                                                                  : possibleSymbol.show(gs);
                            e.addErrorLine(possibleSymbol.loc(gs), "Did you mean: `{}`", suggestedName);
                            continue;
                        }

                        if (possibleSymbol.isClassOrModule()) {
                            if (possibleSymbol.asClassOrModuleRef().data(gs)->typeArity(gs) > 0) {
                                // If this call was in type syntax, we might have already have built an
                                // autocorrect to turn this from `MyClass(...)` to `MyClass[...]`.
                                continue;
                            }
                            e.addErrorNote("Ruby uses `.new` to invoke a class's constructor");
                            e.replaceWith("Insert `.new`", args.funLoc().copyEndWithZeroLength(), ".new");
                            continue;
                        }

                        const auto replacement = possibleSymbol.name(gs).toString(gs);
                        if (replacement != toReplace) {
                            e.didYouMean(replacement, args.funLoc());
                            e.addErrorLine(possibleSymbol.loc(gs), "Defined here");
                            continue;
                        }

                        auto possibleSymbolOwner = possibleSymbol.asMethodRef().data(gs)->owner;
                        if (possibleSymbolOwner.data(gs)->lookupSingletonClass(gs) == symbol) {
                            auto defKeyword = "def ";
                            auto defKeywordLen = char_traits<char>::length(defKeyword);
                            auto prefixLen = defKeywordLen + toReplace.size();
                            auto declLocPrefix = possibleSymbol.loc(gs).adjustLen(gs, 0, prefixLen);
                            e.addErrorNote("Did you mean to define `{}` as a singleton class method?", toReplace);
                            if (declLocPrefix.source(gs) == fmt::format("{}{}", defKeyword, toReplace)) {
                                e.replaceWith("Define method with `self.`",
                                              possibleSymbol.loc(gs).adjustLen(gs, defKeywordLen, 0), "self.");
                            } else {
                                e.addErrorLine(possibleSymbol.loc(gs), "`{}` defined here", toReplace);
                            }
                        }
                    }
                }
            }
        }
        result.main.errors.emplace_back(e.build());
        return result;
    }

    auto method = mayBeOverloaded.data(gs)->flags.isOverloaded
                      ? guessOverload(gs, symbol, mayBeOverloaded, args.numPosArgs, args.args, targs, args.block)
                      : mayBeOverloaded;

    if (method.data(gs)->flags.isPrivate && !args.isPrivateOk) {
        if (auto e = gs.beginError(errLoc, core::errors::Infer::PrivateMethod)) {
            if (args.fullType.type != args.thisType) {
                e.setHeader("Non-private call to private method `{}` on `{}` component of `{}`",
                            method.data(gs)->name.show(gs), args.thisType.show(gs), args.fullType.type.show(gs));
            } else {
                e.setHeader("Non-private call to private method `{}` on `{}`", method.data(gs)->name.show(gs),
                            args.thisType.show(gs));
            }
            e.addErrorLine(method.data(gs)->loc(), "Defined in `{}` here", method.data(gs)->owner.show(gs));
        }
    }

    DispatchResult result;
    auto &component = result.main;
    component.receiver = args.selfType;
    component.method = method;

    auto data = method.data(gs);
    unique_ptr<TypeConstraint> &maybeConstraint = result.main.constr;
    TypeConstraint *constr;
    if (args.block || data->flags.isGenericMethod) {
        maybeConstraint = make_unique<TypeConstraint>();
        constr = maybeConstraint.get();
    } else {
        constr = &TypeConstraint::EmptyFrozenConstraint;
    }

    if (data->flags.isGenericMethod) {
        constr->defineDomain(gs, data->typeArguments());
    }
    auto posArgs = args.numPosArgs;
    bool hasKwargs = absl::c_any_of(data->arguments, [](const auto &arg) { return arg.flags.isKeyword; });
    auto nonPosArgs = (args.args.size() - args.numPosArgs);
    bool hasKwsplat = nonPosArgs & 0x1;
    auto numKwargs = hasKwsplat ? nonPosArgs - 1 : nonPosArgs;

    // p -> params, i.e., what was mentioned in the definition
    auto pit = data->arguments.begin();
    auto pend = data->arguments.end();

    ENFORCE(pit != pend, "Should at least have the block arg.");
    ENFORCE((pend - 1)->flags.isBlock, "Last arg should be the block arg: {}", (pend - 1)->show(gs));
    // We'll type check the block arg separately from the rest of the args.
    --pend;

    // a -> args, i.e., what was passed at the call site
    auto ait = args.args.begin();
    auto aend = args.args.end();
    auto aPosEnd = args.args.begin() + args.numPosArgs;

    while (pit != pend && ait != aPosEnd) {
        const ArgInfo &spec = *pit;
        auto &arg = *ait;
        if (spec.flags.isKeyword) {
            break;
        }
        if (ait + 1 == aend && hasKwargs && (spec.flags.isDefault || spec.flags.isRepeated) &&
            Types::approximate(gs, arg->type, *constr).derivesFrom(gs, Symbols::Hash())) {
            break;
        }

        auto offset = ait - args.args.begin();
        if (auto e = matchArgType(gs, *constr, args.receiverLoc(), symbol, method, *arg, spec, args.selfType, targs,
                                  args.argLoc(offset), args.originForUninitialized, args.args.size() == 1)) {
            result.main.errors.emplace_back(std::move(e));
        }

        if (!spec.flags.isRepeated) {
            ++pit;
        }
        ++ait;
    }

    // If positional arguments remain, the method accepts keyword arguments, and no keyword arguments were provided in
    // the send, assume that the last argument is an implicit keyword args hash.
    bool implicitKwsplat = false;
    if (ait != aPosEnd && hasKwargs && args.args.size() == args.numPosArgs) {
        auto splatLoc = args.argLoc(args.args.size() - 1);

        // If --experimental-ruby3-keyword-args is set, we will treat "**-less" keyword hash argument as an error.
        if (gs.ruby3KeywordArgs) {
            if (auto e = gs.beginError(splatLoc, errors::Infer::KeywordArgHashWithoutSplat)) {
                e.setHeader("Keyword argument hash without `{}` is deprecated", "**");
                e.addErrorLine(splatLoc, "This produces a runtime warning in Ruby 2.7, "
                                         "and will be an error in Ruby 3.0");
                if (auto source = splatLoc.source(gs)) {
                    e.replaceWith(fmt::format("Use `{}` for the keyword argument hash", "**"), splatLoc, "**{}",
                                  source.value());
                }
            }
        }
        hasKwsplat = true;
        implicitKwsplat = true;
    }

    // Extract the kwargs hash if there are keyword args present in the send
    TypePtr kwargs;
    Loc kwargsLoc;
    if (numKwargs > 0 || hasKwsplat) {
        // for cases where the method accepts keyword arguments, none were given, but more positional arguments were
        // given than were expected, just take the location from the last argument of the keyword args list.
        if (numKwargs == 0) {
            kwargsLoc = Loc{args.locs.file, args.locs.args.back()};
        } else {
            auto locStart = args.locs.args[args.numPosArgs];
            auto locEnd = args.locs.args.back();
            kwargsLoc = Loc{args.locs.file, locStart.join(locEnd)};
        }

        vector<TypePtr> keys;
        vector<TypePtr> values;

        // process inlined keyword arguments
        {
            auto kwit = args.args.begin() + args.numPosArgs;
            auto kwend = args.args.begin() + args.numPosArgs + numKwargs;

            while (kwit != kwend) {
                // if the key isn't a symbol literal, break out as this is not a valid keyword
                auto &key = *kwit++;
                if (!isa_type<NamedLiteralType>(key->type) ||
                    cast_type_nonnull<NamedLiteralType>(key->type).literalKind !=
                        NamedLiteralType::LiteralTypeKind::Symbol) {
                    // it's not possible to tell if this is hash will be used as kwargs yet, so we can't raise a useful
                    // error here.

                    keys.clear();
                    values.clear();
                    break;
                }

                auto &val = *kwit++;
                keys.emplace_back(key->type);
                values.emplace_back(val->type);
            }
        }

        // Merge in the keyword splat argument if it's present.
        // Note that we rely on this merging being done after the explicitly-named
        // arguments in the call when determining whether keyword args are properly
        // typed, below.
        bool kwSplatIsHash = false;
        TypePtr kwSplatType;
        if (hasKwsplat) {
            auto &kwSplatArg = *(aend - 1);
            kwSplatType = Types::approximate(gs, kwSplatArg->type, *constr);

            if (hasKwargs) {
                if (auto *hash = fromKwargsHash(gs, kwSplatType)) {
                    absl::c_copy(hash->keys, back_inserter(keys));
                    absl::c_copy(hash->values, back_inserter(values));
                    kwargs = make_type<ShapeType>(move(keys), move(values));
                    --aend;
                } else {
                    if (kwSplatType.isUntyped()) {
                        // Allow an untyped arg to satisfy all kwargs
                        --aend;
                        kwargs = Types::untypedUntracked();
                    } else if (kwSplatType.derivesFrom(gs, Symbols::Hash())) {
                        // This will be an error if the kwsplat hash ends up being used to supply keyword arguments,
                        // however it may also be consumed as a positional arg. Defer raising an error until we're
                        // certain that it would be used as a keyword args hash below.
                        kwSplatIsHash = true;

                        --aend;
                        kwargs = Types::untypedUntracked();
                    }
                }

                // Check to see if the keyword splat was a valid kwargs hash, and consume a positional argument if it
                // was implicit.
                if (implicitKwsplat && kwargs != nullptr) {
                    --posArgs;
                }
            } else {
                // This function doesn't take keyword arguments, so consume the kwsplat and use the approximated type.
                kwargs = kwSplatType;
                --aend;
            }
        } else {
            kwargs = make_type<ShapeType>(move(keys), move(values));
        }

        // Detect the case where not all positional arguments were supplied, causing the keyword args to be consumed as
        // a positional hash.
        if (kwargs != nullptr && pit != pend && !pit->flags.isBlock &&
            (!hasKwargs || (!pit->flags.isRepeated && !pit->flags.isKeyword && !pit->flags.isDefault))) {
            // TODO(trevor) if `hasKwargs` is true at this point but not keyword args were provided, we could add an
            // autocorrect to turn this into `**kwargs`

            // If there are positional arguments left to be filled, but there were keyword arguments present,
            // consume the keyword args hash as though it was a positional arg.
            if (auto e = matchArgType(gs, *constr, args.receiverLoc(), symbol, method,
                                      TypeAndOrigins{kwargs, {kwargsLoc}}, *pit, args.selfType, targs, kwargsLoc,
                                      args.originForUninitialized, args.args.size() == 1)) {
                result.main.errors.emplace_back(std::move(e));
            }

            if (!pit->flags.isRepeated) {
                pit++;
            }

            // Clear out the kwargs hash so that no keyword argument processing is triggered below, and also mark
            // the keyword args as consumed when this method does not accept keyword arguments.
            kwargs = nullptr;
            posArgs++;
            if (!hasKwargs) {
                ait += numKwargs;
            }
        } else if (kwSplatIsHash) {
            const ArgInfo *kwSplatParam = nullptr;
            auto hasRequiredKwParam = false;
            auto kwParams = vector<const ArgInfo *>{};
            for (auto &param : data->arguments) {
                if (param.flags.isKeyword && param.flags.isRepeated) {
                    ENFORCE(kwSplatParam == nullptr);
                    kwSplatParam = &param;
                } else if (param.flags.isKeyword && !param.flags.isRepeated) {
                    kwParams.emplace_back(&param);
                    hasRequiredKwParam |= !param.flags.isDefault;
                }
            }

            auto hasKwSplatParam = kwSplatParam != nullptr;
            auto hasKwParam = !kwParams.empty();

            auto &kwSplatArg = *aend;
            auto kwSplatTPO = TypeAndOrigins{kwSplatType, kwSplatArg->origins};
            auto kwSplatArgLoc = args.argLoc(args.args.size() - 1);

            if (hasKwSplatParam && !hasKwParam) {
                if (auto e = matchArgType(gs, *constr, args.receiverLoc(), symbol, method, kwSplatTPO, *kwSplatParam,
                                          args.selfType, targs, kwargsLoc, args.originForUninitialized,
                                          args.args.size() == 1)) {
                    result.main.errors.emplace_back(std::move(e));
                }
            } else if (hasKwParam) {
                auto hashType = isa_type<ShapeType>(kwSplatType) ? kwSplatType.underlying(gs) : kwSplatType;
                auto &appliedType = cast_type_nonnull<AppliedType>(hashType);
                auto kwSplatKeyType = appliedType.targs[0];
                auto kwSplatValueType = appliedType.targs[1];

                if (hasRequiredKwParam) {
                    if (auto e = gs.beginError(kwSplatArgLoc, errors::Infer::UntypedSplat)) {
                        // Unfortunately, this prevents even valid splats:
                        //     def f(x:, y: 0); end
                        //     f(x: 0, **opts)
                        // This should work, but we currently handle kwsplat even before we check
                        // which keyword args have already been consumed (see below for that).
                        // This is harder to support, but maybe also less likely:
                        //     f(**opts, x: 0)
                        e.setHeader("Cannot call `{}` with a `{}` keyword splat because the method has required "
                                    "keyword parameters",
                                    method.show(gs), "Hash");
                        e.addErrorLine(kwParams.front()->loc,
                                       "Keyword parameters of `{}` begin here:", method.show(gs));
                        e.addErrorSection(kwSplatTPO.explainGot(gs, args.originForUninitialized));
                        e.addErrorNote("Note that Sorbet does not yet handle mixing explicitly-passed keyword args "
                                       "with splats.\n"
                                       "    To ignore this and pass the splat anyways, use `{}`",
                                       "T.unsafe");
                        maybeSuggestUnsafeKwsplat(gs, e, kwSplatArgLoc);
                        result.main.errors.emplace_back(e.build());
                    }
                } else if (!Types::isSubTypeUnderConstraint(gs, *constr, kwSplatKeyType, Types::Symbol(),
                                                            UntypedMode::AlwaysCompatible)) {
                    // TODO(jez) Highlight untyped code for this error
                    if (auto e = gs.beginError(kwSplatArgLoc, errors::Infer::MethodArgumentMismatch)) {
                        e.setHeader("Expected `{}` but found `{}` for keyword splat keys type", "Symbol",
                                    kwSplatKeyType.show(gs));
                        e.addErrorSection(kwSplatTPO.explainGot(gs, args.originForUninitialized));
                        if (gs.suggestUnsafe.has_value()) {
                            maybeSuggestUnsafeKwsplat(gs, e, kwSplatArgLoc);
                        }
                        result.main.errors.emplace_back(e.build());
                    }
                } else {
                    for (const auto &kwParam : kwParams) {
                        auto kwParamType =
                            Types::resultTypeAsSeenFrom(gs, kwParam->type, method.data(gs)->owner, symbol, targs);
                        if (kwParamType == nullptr) {
                            kwParamType = Types::untyped(method);
                        }
                        // TODO(jez) Highlight untyped code for this error
                        if (Types::isSubTypeUnderConstraint(gs, *constr, kwSplatValueType, kwParamType,
                                                            UntypedMode::AlwaysCompatible)) {
                            continue;
                        }

                        if (auto e = gs.beginError(kwSplatArgLoc, errors::Infer::MethodArgumentMismatch)) {
                            e.setHeader("Expected `{}` for keyword parameter `{}` but found `{}` from keyword splat",
                                        kwParamType.show(gs), kwParam->argumentName(gs), kwSplatValueType.show(gs));
                            auto for_ = ErrorColors::format("argument `{}` of method `{}`", kwParam->argumentName(gs),
                                                            method.show(gs));
                            e.addErrorSection(TypeAndOrigins::explainExpected(gs, kwParamType, kwParam->loc, for_));
                            e.addErrorSection(kwSplatTPO.explainGot(gs, args.originForUninitialized));
                            e.addErrorNote(
                                "A `{}` passed as a keyword splat must match the type of all keyword parameters\n"
                                "    because Sorbet cannot see what specific keys exist in the `{}`.",
                                "Hash", "Hash");
                            maybeSuggestUnsafeKwsplat(gs, e, kwSplatArgLoc);
                            result.main.errors.emplace_back(e.build());
                        }

                        break;
                    }
                }
            } else {
                Exception::raise("Code at {} triggered unknown codepath in Sorbet. Please report a bug.",
                                 args.callLoc().showRaw(gs));
            }
        }
    }

    if (pit != pend) {
        if (!(pit->flags.isKeyword || pit->flags.isDefault || pit->flags.isRepeated || pit->flags.isBlock)) {
            if (auto e = gs.beginError(args.argsLoc(), errors::Infer::MethodArgumentCountMismatch)) {
                if (args.fullType.type != args.thisType) {
                    e.setHeader("Not enough arguments provided for method `{}` on `{}` component of `{}`. "
                                "Expected: `{}`, got: `{}`",
                                method.show(gs), args.thisType.show(gs), args.fullType.type.show(gs),
                                prettyArity(gs, method), posArgs);
                } else {
                    e.setHeader("Not enough arguments provided for method `{}`. Expected: `{}`, got: `{}`",
                                method.show(gs), prettyArity(gs, method), posArgs);
                }
                e.addErrorLine(method.data(gs)->loc(), "`{}` defined here", method.show(gs));
                if (targetName == core::Names::any() &&
                    symbol == core::Symbols::T().data(gs)->lookupSingletonClass(gs)) {
                    e.addErrorNote("If you want to allow any type as an argument, use `{}`", "T.untyped");
                }

                result.main.errors.emplace_back(e.build());
            }
        }
    }

    // keep this around so we know which keyword arguments have been supplied
    UnorderedSet<NameRef> consumed;
    if (hasKwargs) {
        // Remember where the kwargs started
        auto kwait = aPosEnd;
        // Mark the keyword args as consumed
        ait += numKwargs;

        if (auto *hash = cast_type<ShapeType>(kwargs)) {
            // find keyword arguments and advance `pend` before them; We'll walk
            // `kwit` ahead below
            auto kwit = pit;
            while (!kwit->flags.isKeyword) {
                kwit++;
            }
            pend = kwit;

            bool sawKwSplat = false;
            while (kwit != data->arguments.end()) {
                const ArgInfo &spec = *kwit;
                if (spec.flags.isBlock) {
                    break;
                } else if (spec.flags.isRepeated) {
                    for (auto it = hash->keys.begin(); it != hash->keys.end(); ++it) {
                        auto key = cast_type_nonnull<NamedLiteralType>(*it);
                        auto underlying = key.underlying(gs);
                        ClassOrModuleRef klass = cast_type_nonnull<ClassType>(underlying).symbol;
                        if (klass != Symbols::Symbol()) {
                            continue;
                        }

                        NameRef arg = key.asName();
                        if (consumed.contains(arg)) {
                            continue;
                        }
                        consumed.insert(arg);

                        // TODO(trevor) this location could be more precise, as we can track the location of the inlined
                        // keyword arguments separately from the ones that come from the kwsplat
                        auto offset = it - hash->keys.begin();
                        TypeAndOrigins tpe{hash->values[offset], kwargsLoc};
                        if (auto e = matchArgType(gs, *constr, args.receiverLoc(), symbol, method, tpe, spec,
                                                  args.selfType, targs, kwargsLoc, args.originForUninitialized)) {
                            result.main.errors.emplace_back(std::move(e));
                        }
                    }
                    sawKwSplat = true;
                    break;
                }
                ++kwit;

                auto arg = absl::c_find_if(hash->keys, [&](const TypePtr &litType) {
                    if (!isa_type<NamedLiteralType>(litType)) {
                        return false;
                    }
                    auto lit = cast_type_nonnull<NamedLiteralType>(litType);
                    auto underlying = lit.underlying(gs);
                    return cast_type_nonnull<ClassType>(underlying).symbol == Symbols::Symbol() &&
                           lit.asName() == spec.name;
                });
                if (arg == hash->keys.end()) {
                    if (!spec.flags.isDefault) {
                        if (auto e = missingArg(gs, args.argsLoc(), args.receiverLoc(), method, spec, symbol, targs)) {
                            result.main.errors.emplace_back(std::move(e));
                        }
                    }
                    continue;
                }
                consumed.insert(spec.name);
                // We want the precise location of the kwarg that we are processing.
                // To do that, we rely on hash->{keys,values} being added in the order
                // that the kwargs appear in the the call.  We also rely on args from
                // a shapely-typed kwsplat hash (implicit or explicit) being added to
                // hash->{keys,values} after the explicitly-named kwargs in the call.
                auto offset = arg - hash->keys.begin();
                // offset * 2 skips kwargs we have already seen.
                // + 1 moves us to the value being passed.
                ENFORCE(!sawKwSplat);
                auto kwaValue = kwait + (offset * 2) + 1;
                ENFORCE(args.args.size() == args.locs.args.size(), "Send arg and loc vectors did not match in length");
                // This in-bounds check is unusual, but it's necessary for cases
                // where we are processing a kwarg that comes from a non-kwsplat
                // shapely-typed hash.  In such cases, our kwargOffset will appear to
                // be out-of-bounds with respect to the actual number of arguments in
                // the call.  (We could track the location information of kwargs from
                // such a hash, but that would require rewriting a good chunk of this
                // function.)  We actually do something slightly stronger, which is
                // to check whether the thing we're pointing at lives inside the range
                // of passed kwargs in the call.
                //
                // If we encounter this case, we've already setup kwargsLoc to reflect
                // as closely as possible the location of the entire shapely-typed
                // hash, so use that for the "origin" of the argument and say that the
                // argument has no location.  The latter also inhibits type-driven
                // autocorrects from kicking in.
                bool argInBounds = kwaValue < ait;
                auto kwargOffset = kwaValue - args.args.begin();
                auto originLoc = argInBounds ? args.argLoc(kwargOffset) : kwargsLoc;
                auto argLoc = argInBounds ? args.argLoc(kwargOffset) : args.callLoc();
                TypeAndOrigins tpe{hash->values[offset], originLoc};
                if (auto e = matchArgType(gs, *constr, args.receiverLoc(), symbol, method, tpe, spec, args.selfType,
                                          targs, argLoc, args.originForUninitialized)) {
                    result.main.errors.emplace_back(std::move(e));
                }
            }
            for (auto &keyType : hash->keys) {
                auto key = cast_type_nonnull<NamedLiteralType>(keyType);
                auto underlying = key.underlying(gs);
                ClassOrModuleRef klass = cast_type_nonnull<ClassType>(underlying).symbol;
                if (klass == Symbols::Symbol() && consumed.contains(key.asName())) {
                    continue;
                }
                NameRef arg = key.asName();

                if (auto e = gs.beginError(args.callLoc(), errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Unrecognized keyword argument `{}` passed for method `{}`", arg.show(gs),
                                method.show(gs));
                    result.main.errors.emplace_back(e.build());
                }
            }
        } else if (kwargs == nullptr) {
            // The method has keyword arguments, but none were provided. Report an error for each missing argument.
            for (auto &spec : data->arguments) {
                if (!spec.flags.isKeyword || spec.flags.isDefault || spec.flags.isRepeated) {
                    continue;
                }
                if (auto e = missingArg(gs, args.argsLoc(), args.receiverLoc(), method, spec, symbol, targs)) {
                    result.main.errors.emplace_back(std::move(e));
                }
            }
        }
    }

    if (ait != aend) {
        auto arity = arityComponents(gs, method);
        auto maxPossiblePositional = arity.required + arity.optional;

        auto hashArgsCount = (numKwargs > 0 || hasKwsplat) ? 1 : 0;
        auto numArgsGiven = args.numPosArgs + hashArgsCount;
        ENFORCE(maxPossiblePositional < numArgsGiven);

        // It's only the end of this loc that matters, because we're going to join it with maxPossiblePositional.
        core::Loc lastPositionalArg;
        if (hashArgsCount == 0) {
            // No keyword args and no kwsplat, so the mismatch must have been from extra pos args
            lastPositionalArg = args.argLoc(args.numPosArgs - 1);
        } else if (!consumed.empty()) {
            // Either keyword args or kwsplat. If we consumed any keyword args, we'd have reported
            // the message as "unrecognized keyword args", not as too many positional args, so there
            // must be a positional arg.
            lastPositionalArg = args.argLoc(args.numPosArgs - 1);
        } else {
            // Didn't consume any of the keyword args, which means they're being included in the
            // list of positional args. Can just take the last arg in the list.
            lastPositionalArg = args.argLoc(args.locs.args.size() - 1);
        }

        auto extraArgsLoc = args.argLoc(maxPossiblePositional).join(lastPositionalArg);
        if (auto e = gs.beginError(extraArgsLoc, errors::Infer::MethodArgumentCountMismatch)) {
            auto deleteLoc = extraArgsLoc;
            // Heuristic to try to find leading commas. Not actually required for correctness
            // (will still parse even if we delete something but don't find the comma) which is
            // why we're fine with this just hard-coding two common cases (easiest to implement).
            if (extraArgsLoc.adjustLen(gs, -1, 1).source(gs) == ",") {
                deleteLoc = extraArgsLoc.adjust(gs, -1, 0);
            } else if (extraArgsLoc.adjustLen(gs, -2, 2).source(gs) == ", ") {
                deleteLoc = extraArgsLoc.adjust(gs, -2, 0);
            } else if (extraArgsLoc.adjustLen(gs, -1, 1).source(gs) == " ") {
                deleteLoc = extraArgsLoc.adjust(gs, -1, 0);
            }

            if (method == Symbols::BasicObject_initialize()) {
                e.setHeader("Wrong number of arguments for constructor. Expected: `{}`, got: `{}`", 0, numArgsGiven);
                e.addErrorLine(method.data(gs)->loc(), "`{}` defined here", targetName.show(gs));
                e.replaceWith("Delete extra args", deleteLoc, "");
            } else if (!hasKwargs) {
                e.setHeader("Too many arguments provided for method `{}`. Expected: `{}`, got: `{}`", method.show(gs),
                            prettyArity(gs, method), numArgsGiven);
                e.addErrorLine(method.data(gs)->loc(), "`{}` defined here", targetName.show(gs));
                if (!deleteLoc.empty()) {
                    e.replaceWith("Delete extra args", deleteLoc, "");
                }
            } else {
                // if we have keyword arguments, we should print a more informative message: otherwise, we might give
                // people some slightly confusing error messages.

                // print a helpful error message
                e.setHeader("Too many positional arguments provided for method `{}`. Expected: `{}`, got: `{}`",
                            method.show(gs), prettyArity(gs, method), posArgs);
                e.addErrorLine(method.data(gs)->loc(), "`{}` defined here", targetName.show(gs));

                // if there's an obvious first keyword argument that the user hasn't supplied, we can mention it
                // explicitly
                auto firstKeyword = absl::c_find_if(data->arguments, [&consumed](const ArgInfo &arg) {
                    return arg.flags.isKeyword && arg.flags.isDefault && consumed.count(arg.name) == 0;
                });
                if (firstKeyword != data->arguments.end()) {
                    auto possibleArg = firstKeyword->argumentName(gs);
                    e.addErrorLine(args.argLoc(maxPossiblePositional),
                                   "`{}` has optional keyword arguments. Did you mean to provide a value for `{}`?",
                                   method.show(gs), possibleArg);
                    auto howManyExtra = numArgsGiven - hashArgsCount - maxPossiblePositional;
                    if (howManyExtra == 1 && extraArgsLoc.adjustLen(gs, -1, 2).source(gs) != "&:") {
                        e.replaceWith(fmt::format("Prefix with `{}:`", possibleArg), extraArgsLoc.copyWithZeroLength(),
                                      "{}: ", possibleArg);
                    }
                } else if (!deleteLoc.empty()) {
                    e.replaceWith("Delete extra args", deleteLoc, "");
                }
            }
            result.main.errors.emplace_back(e.build());
        }
    }

    if (args.block != nullptr) {
        ENFORCE(!data->arguments.empty(), "Every symbol must at least have a block arg: {}", method.show(gs));
        const auto &bspec = data->arguments.back();
        ENFORCE(bspec.flags.isBlock, "The last symbol must be the block arg: {}", method.show(gs));

        // Only report "does not expect a block" error if the method is defined in a `typed: strict`
        // file or higher and has a sig, which would force the "uses `yield` but does not mention a
        // block parameter" error, so we can use the heuristic about isSyntheticBlockArgument.
        // (Some RBI-only strictness levels are technically higher than strict but don't require
        // having written a sig. This usually manifests as `def foo(*_); end` with no sig in an RBI.)
        //
        // We also have to check whether the blockLoc exists and is not empty. (Sometimes the block
        // can be imaginary, like from a bare `super` call).
        if (data->hasSig() && data->loc().exists()) {
            auto file = data->loc().file();
            auto blockLoc = args.blockLoc(gs);
            if (file.exists() && file.data(gs).strictLevel >= core::StrictLevel::Strict &&
                bspec.isSyntheticBlockArgument() && blockLoc.exists() && !blockLoc.empty()) {
                if (auto e = gs.beginError(blockLoc, core::errors::Infer::TakesNoBlock)) {
                    e.setHeader("Method `{}` does not take a block", method.show(gs));
                    for (const auto loc : method.data(gs)->locs()) {
                        e.addErrorLine(loc, "`{}` defined here", method.show(gs));
                    }

                    if (blockLoc.exists()) {
                        e.replaceWith("Remove block", blockLoc, "");
                    }
                }
            }
        }

        TypePtr blockType = Types::resultTypeAsSeenFrom(gs, bspec.type, data->owner, symbol, targs);
        if (!blockType) {
            blockType = Types::untyped(method);
        }

        component.blockReturnType = Types::getProcReturnType(gs, Types::dropNil(gs, blockType));
        blockType = constr->isSolved() ? Types::instantiate(gs, blockType, *constr)
                                       : Types::approximate(gs, blockType, *constr);
        component.blockPreType = blockType;
        component.blockSpec = bspec.deepCopy();
    }

    TypePtr &resultType = result.returnType;

    auto *intrinsic = method.data(gs)->getIntrinsic();
    if (intrinsic != nullptr) {
        intrinsic->apply(gs, args, result);
        // the call could have overridden constraint
        if (result.main.constr || constr != &core::TypeConstraint::EmptyFrozenConstraint) {
            constr = result.main.constr.get();
        }
        if (constr == nullptr) {
            constr = &core::TypeConstraint::EmptyFrozenConstraint;
        }
    }

    if (resultType == nullptr) {
        if (args.args.size() == 1 && method.data(gs)->name.isSetter(gs)) {
            // assignments always return their right hand side
            resultType = args.args.front()->type;
        } else if (args.args.size() == 2 && method.data(gs)->name == Names::squareBracketsEq()) {
            resultType = args.args[1]->type;
        } else {
            resultType =
                Types::resultTypeAsSeenFrom(gs, method.data(gs)->resultType, method.data(gs)->owner, symbol, targs);
        }
    }
    if (args.block == nullptr) {
        // if block is there we do not attempt to solve the constraint. CFG adds an explicit solve
        // node that triggers constraint solving
        if (!constr->solve(gs)) {
            if (auto e = gs.beginError(errLoc, errors::Infer::GenericMethodConstraintUnsolved)) {
                e.setHeader("Could not find valid instantiation of type parameters for `{}`", method.show(gs));
                e.addErrorLine(method.data(gs)->loc(), "`{}` defined here", method.show(gs));
                e.addErrorSection(constr->explain(gs));
                result.main.errors.emplace_back(e.build());
            }
            // This mimics the behavior of the SolveConstraint case in processBinding
            resultType = Types::untypedUntracked();
        }
        ENFORCE(!data->arguments.empty(), "Every method should at least have a block arg.");
        ENFORCE(data->arguments.back().flags.isBlock, "The last arg should be the block arg.");
        auto blockType = data->arguments.back().type;
        // TODO(jez) Highlight untyped code for this error
        if (blockType && !core::Types::isSubType(gs, core::Types::nilClass(), blockType)) {
            if (auto e = gs.beginError(args.callLoc().copyEndWithZeroLength(), errors::Infer::BlockNotPassed)) {
                e.setHeader("`{}` requires a block parameter, but no block was passed", targetName.show(gs));
                e.addErrorLine(method.data(gs)->loc(), "defined here");
                result.main.errors.emplace_back(e.build());
            }
        }
    }

    if (!resultType) {
        resultType = Types::untyped(method);
    } else if (!constr->isEmpty() && constr->isSolved()) {
        resultType = Types::instantiate(gs, resultType, *constr);
    }
    resultType = Types::replaceSelfType(gs, resultType, args.selfType);

    if (args.block != nullptr) {
        component.sendTp = resultType;
    }
    return result;
}

TypePtr getMethodArguments(const GlobalState &gs, ClassOrModuleRef klass, NameRef name, const vector<TypePtr> &targs) {
    MethodRef method = klass.data(gs)->findMethodTransitive(gs, name);

    if (!method.exists()) {
        return core::Types::untypedUntracked();
    }
    auto data = method.data(gs);

    vector<TypePtr> args;
    args.reserve(data->arguments.size());
    for (const auto &arg : data->arguments) {
        if (arg.flags.isRepeated) {
            ENFORCE(args.empty(), "getCallArguments with positional and repeated args is not supported: {}",
                    method.toString(gs));
            return Types::arrayOf(gs, Types::resultTypeAsSeenFrom(gs, arg.type, data->owner, klass, targs));
        }
        ENFORCE(!arg.flags.isKeyword, "getCallArguments does not support kwargs: {}", method.toString(gs));
        if (arg.flags.isBlock) {
            continue;
        }
        if (arg.type == nullptr) {
            args.emplace_back(core::Types::untyped(method));
            continue;
        }
        args.emplace_back(Types::resultTypeAsSeenFrom(gs, arg.type, data->owner, klass, targs));
    }
    return make_type<TupleType>(move(args));
}
} // namespace

DispatchResult ClassType::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    categoryCounterInc("dispatch_call", "classtype");
    vector<TypePtr> empty;
    return dispatchCallSymbol(gs, args, symbol, empty);
}

DispatchResult AppliedType::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    categoryCounterInc("dispatch_call", "appliedType");
    return dispatchCallSymbol(gs, args, this->klass, this->targs);
}

TypePtr ClassType::getCallArguments(const GlobalState &gs, NameRef name) const {
    if (symbol == core::Symbols::untyped()) {
        return Types::untyped(Symbols::noSymbol());
    }
    return getMethodArguments(gs, symbol, name, vector<TypePtr>{});
}

TypePtr BlamedUntyped::getCallArguments(const GlobalState &gs, NameRef name) const {
    // BlamedUntyped are always untyped.
    return Types::untyped(blame);
}

TypePtr AppliedType::getCallArguments(const GlobalState &gs, NameRef name) const {
    return getMethodArguments(gs, klass, name, targs);
}

namespace {

// Determines whether we will allow `new` on a type wrapped by a `MetaType`. Note that this function is conservative,
// in that there are some cases we want to reject but cannot detect here.
bool canCallNew(const GlobalState &gs, const TypePtr &wrapped) {
    if (isa_type<OrType>(wrapped) || isa_type<AndType>(wrapped)) {
        return false;
    }

    if (isa_type<ClassType>(wrapped)) {
        auto sym = cast_type_nonnull<ClassType>(wrapped).symbol;
        if (sym == Symbols::untyped() || sym == Symbols::bottom()) {
            return false;
        } else if (sym.data(gs)->isSingletonClass(gs)) {
            return false;
        }
    }

    if (auto *appliedType = cast_type<AppliedType>(wrapped)) {
        if (appliedType->klass == core::Symbols::Class()) {
            // T::Class[...].new is not implemented--users should just use Class.new(super_class)
            return false;
        }

        if (appliedType->klass.data(gs)->isSingletonClass(gs)) {
            return false;
        }

        auto klassId = appliedType->klass.id();
        if (core::Symbols::Proc(0).id() <= klassId && klassId <= core::Symbols::last_proc().id()) {
            return false;
        }
    }

    return true;
}

DispatchResult badMetaTypeCall(const GlobalState &gs, const DispatchArgs &args, core::Loc errLoc,
                               const TypePtr &wrapped) {
    if (auto e = gs.beginError(errLoc, errors::Infer::MetaTypeDispatchCall)) {
        e.setHeader("Call to method `{}` on `{}` mistakes a type for a value", args.name.show(gs), wrapped.show(gs));

        if (isa_type<SelfTypeParam>(wrapped)) {
            auto selfTypeParam = cast_type_nonnull<SelfTypeParam>(wrapped);
            if (selfTypeParam.definition.isTypeMember()) {
                e.addErrorNote("Sorbet erases all generics, so `{}` is never a meaningful, concrete type at runtime.\n"
                               "    If you want to call a method on some class, that class object must be an argument "
                               "to this method.",
                               selfTypeParam.show(gs));
            }
        }

        if (args.name == core::Names::tripleEq()) {
            if (auto appliedType = cast_type<AppliedType>(wrapped)) {
                e.addErrorNote("It looks like you're trying to pattern match on a generic, "
                               "which doesn't work at runtime");
                e.replaceWith("Replace with class name", args.callLoc(), "{}", appliedType->klass.show(gs));
            }
        } else if (auto *appliedType = cast_type<AppliedType>(wrapped)) {
            // For T.class_of(Foo), we'll suggest replacing it with the attached class (Foo).
            if (appliedType->klass.data(gs)->isSingletonClass(gs)) {
                auto receiverLoc = core::Loc(args.locs.file, args.locs.receiver);
                e.replaceWith("Replace with class name", receiverLoc, "{}",
                              appliedType->klass.data(gs)->attachedClass(gs).show(gs));
            } else if (appliedType->klass == core::Symbols::Class()) {
                e.addErrorNote("Sorbet erases generics, so `{}` does not work. Use `{}` instead.", "T::Class[...].new",
                               "Class.new(...)");
            }
        }
    }
    return DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noMethod());
}

} // namespace

DispatchResult MetaType::dispatchCall(const GlobalState &gs, const DispatchArgs &args) const {
    auto errLoc = args.errLoc();
    switch (args.name.rawId()) {
        case Names::new_().rawId(): {
            if (!canCallNew(gs, wrapped)) {
                badMetaTypeCall(gs, args, errLoc, wrapped);
                return DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noMethod());
            }

            // The Ruby VM treats `initialize` as private by default, but allows calling it directly within `new`.
            auto innerArgs = DispatchArgs{Names::initialize(),
                                          args.locs,
                                          args.numPosArgs,
                                          args.args,
                                          wrapped,
                                          {wrapped, args.fullType.origins},
                                          wrapped,
                                          args.block,
                                          args.originForUninitialized,
                                          /* isPrivateOk */ true,
                                          args.suppressErrors,
                                          args.enclosingMethodForSuper};
            auto original = wrapped.dispatchCall(gs, innerArgs);
            original.returnType = wrapped;
            original.main.sendTp = wrapped;
            return original;
        }
        case Names::squareBrackets().rawId(): {
            auto *applied = cast_type<AppliedType>(this->wrapped);
            if (applied == nullptr) {
                return badMetaTypeCall(gs, args, errLoc, this->wrapped);
            }

            auto returnType = Types::applyTypeArguments(gs, args.locs, args.numPosArgs, args.args, applied->klass);
            return DispatchResult(returnType, args.selfType, Symbols::T_Generic_squareBrackets());
        }
        case Names::bind().rawId():
        case Names::returns().rawId():
        case Names::void_().rawId(): {
            auto *applied = cast_type<AppliedType>(this->wrapped);
            if (applied == nullptr) {
                return badMetaTypeCall(gs, args, errLoc, this->wrapped);
            }

            auto klassId = applied->klass.id();
            if (!(core::Symbols::Proc(0).id() <= klassId && klassId <= core::Symbols::last_proc().id())) {
                return badMetaTypeCall(gs, args, errLoc, this->wrapped);
            }

            if (args.name == Names::bind()) {
                // Sometimes people put T.proc.void.bind(...) instead of T.proc.bind(...).void in a signature
                auto member = core::Symbols::T_Private_Methods_DeclBuilder().data(gs)->findMember(gs, args.name);
                // might not exist if running with --no-stdlib
                auto method = member.exists() ? member.asMethodRef() : core::Symbols::noMethod();
                return DispatchResult(args.selfType, args.selfType, method);
            }

            if (applied->targs.size() < 1 || applied->targs[0] != core::Types::todo()) {
                return badMetaTypeCall(gs, args, errLoc, this->wrapped);
            }

            if (args.name == Names::returns() && args.args.size() != 1 && args.numPosArgs != 1) {
                return badMetaTypeCall(gs, args, errLoc, this->wrapped);
            }

            auto returns = core::Types::void_();
            if (args.name == core::Names::returns()) {
                returns = Types::unwrapType(gs, args.argLoc(0), args.args[0]->type);
            }

            // Have to create a new type for the result because dispatchCall is a const member method
            vector<core::TypePtr> targs;

            // Going to overwrite the return type later in MetaType::dispatchCall
            targs.emplace_back(returns);
            for (size_t i = 1; i < applied->targs.size(); i++) {
                targs.emplace_back(applied->targs[i]);
            }

            auto proc = make_type<MetaType>(core::make_type<core::AppliedType>(applied->klass, move(targs)));

            auto member = core::Symbols::T_Private_Methods_DeclBuilder().data(gs)->findMember(gs, args.name);
            // might not exist if running with --no-stdlib
            auto method = member.exists() ? member.asMethodRef() : core::Symbols::noMethod();
            return DispatchResult(proc, std::move(args.selfType), method);
        }
        case Names::valid_p().rawId():
        case Names::recursivelyValid_p().rawId():
        case Names::subtypeOf_p().rawId():
        case Names::describeObj().rawId():
        case Names::errorMessageForObj().rawId():
        case Names::errorMessageForObjRecursive().rawId():
        case Names::validate_bang().rawId(): {
            // These are methods on `T::Types::Base`. Don't report an error, but also for the time
            // being don't even attempt to type the result properly. We can break these out and
            // handle them better in the future if we want to.
            return DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noMethod());
        }
        default:
            return badMetaTypeCall(gs, args, errLoc, this->wrapped);
    }
}

namespace {

// @param mustExist Whether to ENFORCE on failure. If false, the returned ClassOrModuleRef will not
//   exist. (Passing mustExist is largely the same as ENFORCE'ing `exists()` at the call site, but
//   the error message is slightly better, because it mentions the bad type).
//
//   When `mustExist == false`, the caller must take care to check whether the result exists.
ClassOrModuleRef unwrapSymbol(const GlobalState &gs, const TypePtr &type, bool mustExist) {
    ClassOrModuleRef result;
    TypePtr typePtr = type;
    bool breakOut = false;
    while (!result.exists() && !breakOut) {
        const TypePtr &ptrForTypecase = typePtr;
        typecase(
            ptrForTypecase,

            [&](const ClassType &klass) { result = klass.symbol; },

            [&](const AppliedType &app) { result = app.klass; },

            [&](const TypePtr &ty) {
                if (is_proxy_type(ty)) {
                    typePtr = ty.underlying(gs);
                } else {
                    if (mustExist) {
                        ENFORCE(false, "Unexpected type: {}", ty.typeName());
                    }
                    breakOut = true;
                }
            });
    }
    return result;
}

class T_untyped : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        res.returnType = make_type<MetaType>(Types::untypedUntracked());
    }
} T_untyped;

class T_noreturn : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        res.returnType = make_type<MetaType>(Types::bottom());
    }
} T_noreturn;

class T_anything : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        res.returnType = make_type<MetaType>(Types::top());
    }
} T_anything;

class T_class_of : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() != 1 && args.locs.args.size() != 1) {
            // Arity mismatch error was already reported. Fail gracefully
            return;
        }

        // The argument to `T.class_of(...)` is a value, but has a type meaning. That means we need
        // to  `unwrapType` to handle things like type aliases and constant literal types.
        auto unwrappedType = Types::unwrapType(gs, args.argLoc(0), args.args[0]->type);
        auto mustExist = false;
        auto classSymbol = unwrapSymbol(gs, unwrappedType, mustExist);
        if (!classSymbol.exists()) {
            // Non-class type in `T.class_of` is an error that was already reported in type syntax parsing.
            return;
        }

        auto classOf = classSymbol.data(gs)->lookupSingletonClass(gs);
        if (classOf.exists()) {
            res.returnType = make_type<MetaType>(classOf.data(gs)->externalType());
        }
    }
} T_class_of;

class T_self_type : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        res.returnType = make_type<MetaType>(Types::untypedUntracked());
    }
} T_self_type;

class T_attached_class : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // Most syntactically visible calls to `T.attached_class` should be handled by the
        // `<Magic>.attached_class` intrinsic.
        //
        // This intrinsic remains just on the off chance that people misuse `T.attached_class` as a
        // value some other way (e.g., t = T; t.attached_class).
        res.returnType = make_type<MetaType>(Types::untypedUntracked());
    }
} T_attached_class;

class T_type_parameter : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // Proper typing of this should have been in builder_walk by translating `T.type_parameter`
        // calls to `Alias` instructions. This just makes it so that e.g. sigs that use
        // `T.type_parameter(:U)` can be used at `# typed: strong`
        res.returnType = make_type<MetaType>(Types::untypedUntracked());
    }
} T_type_parameter;

class T_must : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.empty()) {
            return;
        }

        auto methodName = args.name == Names::mustBecause() ? "T.must_because" : "T.must";

        if (!args.args[0]->type.isFullyDefined()) {
            if (auto e = gs.beginError(args.argLoc(0), errors::Infer::BareTypeUsage)) {
                e.setHeader("`{}` applied to incomplete type `{}`", methodName, args.args[0]->type.show(gs));
            }
            return;
        }
        auto ret = Types::dropNil(gs, args.args[0]->type);
        if (ret == args.args[0]->type) {
            auto code = args.args[0]->type.isUntyped() ? errors::Infer::MustOnUntyped : errors::Infer::InvalidCast;
            if (auto e = gs.beginError(args.argLoc(0), code)) {
                if (code == errors::Infer::MustOnUntyped) {
                    e.setHeader("`{}` called on `{}`, which is redundant", methodName, args.args[0]->type.show(gs));
                } else {
                    e.setHeader("`{}` called on `{}`, which is never `{}`", methodName, args.args[0]->type.show(gs),
                                "nil");
                }
                e.addErrorSection(args.args[0]->explainGot(gs, args.originForUninitialized));
                auto replaceLoc = args.callLoc();
                const auto locWithoutTMust = args.argLoc(0);
                if (replaceLoc.exists() && locWithoutTMust.exists()) {
                    e.replaceWith(fmt::format("Remove redundant `{}`", methodName), replaceLoc, "{}",
                                  locWithoutTMust.source(gs).value());
                }
            }
        }
        res.returnType = move(ret);
    }
} T_must;

class T_any : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.empty()) {
            return;
        }

        TypePtr ret = Types::bottom();
        auto i = -1;
        for (auto &arg : args.args) {
            i++;
            auto ty = Types::unwrapType(gs, args.argLoc(i), arg->type);
            ret = Types::any(gs, ret, ty);
        }

        res.returnType = make_type<MetaType>(move(ret));
    }
} T_any;

class T_all : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.empty()) {
            return;
        }

        TypePtr ret = Types::top();
        auto i = -1;
        for (auto &arg : args.args) {
            i++;
            auto ty = Types::unwrapType(gs, args.argLoc(i), arg->type);
            ret = Types::all(gs, ret, ty);
        }

        res.returnType = make_type<MetaType>(move(ret));
    }
} T_all;

class T_revealType : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }

        if (auto e = gs.beginError(args.callLoc(), errors::Infer::RevealType)) {
            e.setHeader("Revealed type: `{}`", args.args[0]->type.showWithMoreInfo(gs));
            e.addErrorSection(args.args[0]->explainGot(gs, args.originForUninitialized));
        }
        res.returnType = args.args[0]->type;
    }
} T_revealType;

class T_nilable : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }

        res.returnType = make_type<MetaType>(
            Types::any(gs, Types::unwrapType(gs, args.argLoc(0), args.args[0]->type), Types::nilClass()));
    }
} T_nilable;

class T_proc : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // NOTE: real validation done during infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} T_proc;

class T_proc_params : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.numPosArgs != 0 || args.args.size() % 2 == 1) {
            res.returnType = Types::declBuilderForProcsSingletonClass();
            return;
        }

        auto sym = core::Symbols::Proc(args.args.size() / 2);
        if (!sym.exists()) {
            res.returnType = Types::untypedUntracked();
            return;
        }

        vector<core::TypePtr> targs;
        // Going to overwrite the return type later in MetaType::dispatchCall
        targs.emplace_back(core::Types::todo());

        for (size_t i = 1; i < args.args.size(); i += 2) {
            auto unwrappedType = Types::unwrapType(gs, args.argLoc(i), args.args[i]->type);
            targs.emplace_back(move(unwrappedType));
        }

        res.returnType = make_type<MetaType>(core::make_type<core::AppliedType>(sym, move(targs)));
    }
} T_proc_params;

class T_proc_void : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto sym = core::Symbols::Proc(0);
        vector<core::TypePtr> targs;
        targs.emplace_back(core::Types::void_());
        res.returnType = make_type<MetaType>(core::make_type<core::AppliedType>(sym, move(targs)));
    }
} T_proc_void;

class T_proc_returns : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            res.returnType = core::Types::untypedUntracked();
            return;
        }

        auto sym = core::Symbols::Proc(0);
        vector<core::TypePtr> targs;
        auto unwrappedType = Types::unwrapType(gs, args.argLoc(0), args.args[0]->type);
        targs.emplace_back(move(unwrappedType));
        res.returnType = make_type<MetaType>(core::make_type<core::AppliedType>(sym, move(targs)));
    }
} T_proc_returns;

class DeclBuilderForProcs_bind : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // NOTE: real validation done in infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} DeclBuilderForProcs_bind;

class Object_class : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto mustExist = true;
        ClassOrModuleRef self = unwrapSymbol(gs, args.thisType, mustExist);
        auto tClassSelfType = Types::tClass(args.selfType);
        if (self.data(gs)->isModule()) {
            ENFORCE(gs.requiresAncestorEnabled, "Congrats, you've found a test case. Please add it, then delete this.");
            // This normally can't happen, because `Object` is not an ancestor of any module
            // instance by default. But Sorbet supports requires ancestor in a really weird way (by
            // simply dispatching to a completely unrelated method) which means that sometimes we
            // can actually get a call to this on a module.
            //
            // In the case where the receiver is a module, `singleton` will be `T.class_of(MyModule)`
            // which will not actually reflect how `.class` in a module instance method works at runtime.
            // (see https://sorbet.org/docs/class-of#tclass_of-and-modules)
            res.returnType = tClassSelfType;
            return;
        }

        auto singleton = self.data(gs)->lookupSingletonClass(gs);
        if (!singleton.exists()) {
            res.returnType = tClassSelfType;
            return;
        }

        // `singleton` might have more type members than just the `<AttachedClass>` one.
        // Calling `externalType` is the easiest way to get proper defaults for all of those.
        // For the `<AttachedClass>` type member, we'll default it to its upper bound, like
        //     T.class_of(MyClass)[MyClass, ...]
        // Then the `T.all` is the easiest way to narrow *only* the type argument that
        // corresponds to the `<AttachedClass>` type member, because `T.all` has logic to align
        // type members in parent/child classes. The `T.all` gets pushed through and collapsed
        // like normal, and ends up something like
        //     T.class_of(MyClass)[T.all(TypeOfReceiver, MyClass)]
        // (This matters, btw, in case the receiver is something like a generic.)
        res.returnType = Types::all(gs, tClassSelfType, singleton.data(gs)->externalType());
    }
} Object_class;

class Class_new : public IntrinsicMethod {
public:
    vector<NameRef> dispatchesTo() const override {
        return {core::Names::initialize()};
    }

    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto *selfApp = cast_type<AppliedType>(args.thisType);
        if (selfApp == nullptr) {
            return;
        }

        // If someone takes `klass: T::Class[T.anything]` and calls `klass.new`, the call is
        // actually going to be on an "instance" not a singleton (Class.new, the one on the
        // singleton, is the one that defines a new class at runtime).
        //
        // In that case, `Class` is an instance class (there's no `->attachedClass(gs)` to look for
        // an `initialize` method on). So let's extract out the type applied to the
        // `<AttachedClass>` type member and forward the dispatch to `initialize` on that type.
        //
        // Note that this subsumes cases like calls to new on `T.class_of(MyClass)` because class
        // singleton classes ARE applied types, obviating the need to call `->attachedClass(gs)` at all.
        auto currentAlignment = Types::alignBaseTypeArgs(gs, selfApp->klass, selfApp->targs, Symbols::Class());
        auto it = absl::c_find_if(
            currentAlignment, [&](auto tmRef) { return tmRef.data(gs)->name == Names::Constants::AttachedClass(); });
        ENFORCE(it != currentAlignment.end());
        auto instanceTy = selfApp->targs[distance(currentAlignment.begin(), it)];

        // The Ruby VM treats `initialize` as private by default, but allows calling it directly within `new`.
        DispatchArgs innerArgs{Names::initialize(),
                               args.locs,
                               args.numPosArgs,
                               args.args,
                               instanceTy,
                               {instanceTy, args.fullType.origins},
                               instanceTy,
                               args.block,
                               args.originForUninitialized,
                               /* isPrivateOk */ true,
                               args.suppressErrors,
                               args.enclosingMethodForSuper};
        auto dispatched = instanceTy.dispatchCall(gs, innerArgs);

        // Sorbet *should* treat a missing `initialize` here as a bug, because it can't know the
        // arity/types of the constructor. The most common way `initialize` might be missing is if
        // the user has written `Class`, which is treated like `T::Class[T.anything]`. In that case,
        // the type means "this could be literally any class," and so the right thing to do would be
        // to request a better constructor from the user.
        //
        // But there's an unknown amount of code relying on this right now (from before `T::Class`
        // was built), which in combination with some other bugs, would mean that treating a missing
        // `initialize` as an error would introduce low-value friction until we fix two other bugs:
        //
        // - https://github.com/sorbet/sorbet/issues/1317
        // - https://github.com/sorbet/sorbet/issues/7309
        //
        // More information here:
        // https://github.com/sorbet/sorbet/pull/7285#issuecomment-1718167511
        //
        // There's also the state of things at runtime, described in this comment:
        // https://github.com/sorbet/sorbet/pull/6888/files#diff-b565686f17b8592d5c1e544cd639aaac3244869ac059f2db416a2ac24aa94675R9
        //
        // So for now, simply swallow "missing `initialize`" errors.
        if (dispatched.main.method.exists()) {
            // If we actually dispatched to some `initialize` method, use that method as the result,
            // because it will be more interesting to people downstream who want to look at the
            // result.
            //
            // But if this class hasn't defined a custom `initialize` method, still record that we
            // dispatched to *something*, namely `Class#new`.

            for (auto &err : res.main.errors) {
                dispatched.main.errors.emplace_back(std::move(err));
            }
            res.main = move(dispatched.main);
        }

        res.main.sendTp = instanceTy;
    }
} Class_new;

class Class_subclasses : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        res.returnType = Types::arrayOf(gs, args.thisType);
    }
} Class_subclasses;

class T_Generic_squareBrackets : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto mustExist = true;
        ClassOrModuleRef self = unwrapSymbol(gs, args.thisType, mustExist);
        auto attachedClass = self.data(gs)->attachedClass(gs);

        if (!attachedClass.exists()) {
            return;
        }

        res.returnType = Types::applyTypeArguments(gs, args.locs, args.numPosArgs, args.args, attachedClass);
    }
} T_Generic_squareBrackets;

void applySig(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res, size_t argsToDropOffEnd) {
    // We should always have the actual receiver plus whatever args we're going
    // to ignore for dispatching purposes.
    if (args.args.size() < (argsToDropOffEnd + 1)) {
        return;
    }

    const size_t argsOffset = 1;
    auto callLocsReceiver = args.locs.args[0];
    auto callLocsArgs = InlinedVector<LocOffsets, 2>{};
    for (auto loc = args.locs.args.begin() + argsOffset, end = args.locs.args.end() - argsToDropOffEnd; loc != end;
         ++loc) {
        callLocsArgs.emplace_back(*loc);
    }
    CallLocs callLocs{args.locs.file, args.locs.call, callLocsReceiver, args.locs.fun, callLocsArgs};

    uint16_t numPosArgs = args.numPosArgs - (1 + argsToDropOffEnd);
    auto dispatchArgsArgs = InlinedVector<const TypeAndOrigins *, 2>{};
    for (auto arg = args.args.begin() + argsOffset, end = args.args.end() - argsToDropOffEnd; arg != end; ++arg) {
        dispatchArgsArgs.emplace_back(*arg);
    }

    auto recv = *args.args[0];
    res = recv.type.dispatchCall(gs, {core::Names::sig(), callLocs, numPosArgs, dispatchArgsArgs, recv.type, recv,
                                      recv.type, args.block, args.originForUninitialized, args.isPrivateOk,
                                      args.suppressErrors, args.enclosingMethodForSuper});
}

class SorbetPrivateStatic_sig : public IntrinsicMethod {
public:
    // Forward Sorbet::Private::Static.sig(recv, ...) {...} to recv.sig(...) {...}
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        applySig(gs, args, res, 0);
    }
} SorbetPrivateStatic_sig;

class SorbetPrivateStaticResolvedSig_sig : public IntrinsicMethod {
public:
    // Forward Sorbet::Private::Static::ResolvedSig.sig(recv, ..., <self-method>, <method-name>) {...} to recv.sig(...)
    // {...}
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        const size_t selfAndMethodSymbol = 2;
        applySig(gs, args, res, selfAndMethodSymbol);
    }
} SorbetPrivateStaticResolvedSig_sig;

class Magic_buildHashOrKeywordArgs : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        ENFORCE(args.args.size() % 2 == 0);

        for (int i = 0; i < args.args.size(); i += 2) {
            if (!isa_type<NamedLiteralType>(args.args[i]->type) && !isa_type<IntegerLiteralType>(args.args[i]->type) &&
                !isa_type<FloatLiteralType>(args.args[i]->type)) {
                res.returnType = Types::hashOfUntyped(Symbols::Magic_UntypedSource_buildHash());
                return;
            }
        }

        vector<TypePtr> keys;
        vector<TypePtr> values;
        keys.reserve(args.args.size() / 2);
        values.reserve(args.args.size() / 2);
        for (int i = 0; i < args.args.size(); i += 2) {
            keys.emplace_back(args.args[i]->type);
            values.emplace_back(args.args[i + 1]->type);
        }
        res.returnType = make_type<ShapeType>(move(keys), move(values));
    }
} Magic_buildHashOrKeywordArgs;

class Magic_buildArray : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        vector<TypePtr> elems;
        elems.reserve(args.args.size());
        for (auto &elem : args.args) {
            elems.emplace_back(elem->type);
        }

        res.returnType = make_type<TupleType>(move(elems));
    }
} Magic_buildArray;

class Magic_buildRange : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        ENFORCE(args.args.size() == 3, "Magic_buildRange called with missing arguments");

        auto rangeElemType = Types::dropLiteral(gs, args.args[0]->type);
        auto firstArgIsNil = rangeElemType.isNilClass();
        if (!firstArgIsNil) {
            rangeElemType = Types::dropNil(gs, rangeElemType);
        }
        auto other = Types::dropLiteral(gs, args.args[1]->type);
        auto secondArgIsNil = other.isNilClass();
        if (firstArgIsNil) {
            if (secondArgIsNil) {
                rangeElemType = Types::untyped(Symbols::Magic_UntypedSource_buildRange());
            } else {
                rangeElemType = Types::dropNil(gs, other);
            }
        } else if (!secondArgIsNil) {
            rangeElemType = Types::any(gs, rangeElemType, Types::dropNil(gs, other));
        }
        res.returnType = Types::rangeOf(gs, rangeElemType);
    }
} Magic_buildRange;

class Magic_expandSplat : public IntrinsicMethod {
    static TypePtr expandArray(const GlobalState &gs, const TypePtr &type, int expandTo) {
        if (auto *ot = cast_type<OrType>(type)) {
            return Types::any(gs, expandArray(gs, ot->left, expandTo), expandArray(gs, ot->right, expandTo));
        }

        auto *tuple = cast_type<TupleType>(type);
        if (tuple == nullptr && core::Types::approximate(gs, type, core::TypeConstraint::EmptyFrozenConstraint)
                                    .derivesFrom(gs, Symbols::Array())) {
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

        return make_type<TupleType>(move(types));
    }

public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() != 3) {
            res.returnType = Types::arrayOfUntyped(Symbols::Magic_UntypedSource_expandSplat());
            return;
        }
        auto val = args.args.front()->type;
        if (!(isa_type<IntegerLiteralType>(args.args[1]->type) && isa_type<IntegerLiteralType>(args.args[2]->type))) {
            res.returnType = Types::untyped(Symbols::Magic_UntypedSource_expandSplat());
            return;
        }
        auto &beforeLit = cast_type_nonnull<IntegerLiteralType>(args.args[1]->type);
        auto &afterLit = cast_type_nonnull<IntegerLiteralType>(args.args[2]->type);
        int before = (int)beforeLit.value;
        int after = (int)afterLit.value;
        res.returnType = expandArray(gs, val, before + after);
    }
} Magic_expandSplat;

class Magic_callWithSplat : public IntrinsicMethod {
    friend class Magic_callWithSplatAndBlock;

private:
    static InlinedVector<const TypeAndOrigins *, 2> generateSendArgs(const TupleType *posTuple,
                                                                     const TupleType *kwTuple,
                                                                     InlinedVector<TypeAndOrigins, 2> &sendArgStore,
                                                                     Loc argsLoc) {
        auto numKwArgs = kwTuple != nullptr ? kwTuple->elems.size() : 0;

        sendArgStore.reserve(posTuple->elems.size() + numKwArgs);

        for (auto &arg : posTuple->elems) {
            sendArgStore.emplace_back(arg, argsLoc);
        }

        // kwTuple is a nullptr when there are no keyword args present
        if (kwTuple != nullptr) {
            for (auto &arg : kwTuple->elems) {
                sendArgStore.emplace_back(arg, argsLoc);
            }
        }

        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        sendArgs.reserve(sendArgStore.size());
        for (auto &arg : sendArgStore) {
            sendArgs.emplace_back(&arg);
        }

        return sendArgs;
    }

public:
    // NOTE: Since this method dispatches to one of its Symbol-literal arguments, it has special
    // handling in Substitute.cc to account for fast path updates.
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] are the splat arguments
        // args[3] are the keyword args

        if (args.args.size() != 4) {
            return;
        }

        if (!isa_type<NamedLiteralType>(args.args[1]->type)) {
            return;
        }
        auto lit = cast_type_nonnull<NamedLiteralType>(args.args[1]->type);
        if (!lit.derivesFrom(gs, Symbols::Symbol())) {
            return;
        }

        NameRef fn = lit.asName();

        auto &receiver = args.args[0];
        if (receiver->type.isUntyped()) {
            auto what = core::errors::Infer::errorClassForUntyped(gs, args.locs.file, receiver->type);
            if (auto e = gs.beginError(args.argLoc(0), what)) {
                e.setHeader("Call to method `{}` on `{}`", fn.show(gs), "T.untyped");
                TypeErrorDiagnostics::explainUntyped(gs, e, what, *args.args[0], args.originForUninitialized);
            }

            res.returnType = receiver->type;
            return;
        }

        if (!receiver->type.isFullyDefined()) {
            return;
        }

        if (args.args[2]->type.isUntyped()) {
            auto what = core::errors::Infer::errorClassForUntyped(gs, args.locs.file, args.args[2]->type);
            if (auto e = gs.beginError(args.argLoc(2), what)) {
                e.setHeader("Call to method `{}` with `{}` splat arguments", fn.show(gs), "T.untyped");
                TypeErrorDiagnostics::explainUntyped(gs, e, what, *args.args[2], args.originForUninitialized);
            }

            res.returnType = args.args[2]->type;
            return;
        }
        auto *posTuple = cast_type<TupleType>(args.args[2]->type);
        if (posTuple == nullptr) {
            if (auto e = gs.beginError(args.argLoc(2), core::errors::Infer::UntypedSplat)) {
                e.setHeader("Splats are only supported where the size of the array is known statically");
            }
            return;
        }

        auto kwArgsType = args.args[3]->type;
        auto *kwTuple = cast_type<TupleType>(kwArgsType);
        if (kwTuple == nullptr && !kwArgsType.isNilClass()) {
            if (auto e = gs.beginError(args.argLoc(2), core::errors::Infer::UntypedSplat)) {
                e.setHeader(
                    "Keyword args with splats are only supported where the shape of the hash is known statically");
            }
            return;
        }

        uint16_t numPosArgs = posTuple->elems.size();

        InlinedVector<TypeAndOrigins, 2> sendArgStore;
        InlinedVector<const TypeAndOrigins *, 2> sendArgs =
            Magic_callWithSplat::generateSendArgs(posTuple, kwTuple, sendArgStore, args.argLoc(2));
        InlinedVector<LocOffsets, 2> sendArgLocs(sendArgs.size(), args.locs.args[2]);
        CallLocs sendLocs{args.locs.file, args.locs.call, args.locs.args[0], args.locs.fun, sendArgLocs};
        DispatchArgs innerArgs{fn,
                               sendLocs,
                               numPosArgs,
                               sendArgs,
                               receiver->type,
                               *receiver,
                               receiver->type,
                               args.block,
                               args.originForUninitialized,
                               args.isPrivateOk,
                               args.suppressErrors,
                               args.enclosingMethodForSuper};
        auto dispatched = receiver->type.dispatchCall(gs, innerArgs);
        for (auto &err : dispatched.main.errors) {
            res.main.errors.emplace_back(std::move(err));
        }
        dispatched.main.errors = move(res.main.errors);

        // TODO(trevor) this should merge constraints from `res` and `dispatched` instead
        if ((dispatched.main.constr == nullptr) || dispatched.main.constr->isEmpty()) {
            dispatched.main.constr = move(res.main.constr);
        }
        res = move(dispatched);

        return;
    }
} Magic_callWithSplat;

class Magic_callWithBlock : public IntrinsicMethod {
    friend class Magic_callWithSplatAndBlock;

private:
    static TypePtr typeToProc(const GlobalState &gs, const TypeAndOrigins &blockType, core::FileRef file,
                              LocOffsets callLoc, LocOffsets receiverLoc, LocOffsets funLoc, Loc originForUninitialized,
                              bool suppressErrors) {
        auto nonNilBlockType = blockType;
        auto typeIsNilable = false;
        if (blockType.type.isUntyped()) {
            // Don't simulate a call to `to_proc` on `T.untyped`
            // This avoids reporting a typed: strong error for `&x` where `x` is untyped--we may
            // still want to report an error later when matching this `T.untyped` we're about to
            // return with the method's block parameter.
            return blockType.type;
        }

        if (Types::isSubType(gs, Types::nilClass(), blockType.type)) {
            nonNilBlockType = TypeAndOrigins{Types::dropNil(gs, blockType.type), blockType.origins};
            typeIsNilable = true;

            if (nonNilBlockType.type.isBottom()) {
                return Types::nilClass();
            }
        }

        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        InlinedVector<LocOffsets, 2> sendArgLocs;
        CallLocs sendLocs{file, callLoc, receiverLoc, funLoc, sendArgLocs};
        DispatchArgs innerArgs{core::Names::toProc(),
                               sendLocs,
                               0,
                               sendArgs,
                               nonNilBlockType.type,
                               nonNilBlockType,
                               nonNilBlockType.type,
                               nullptr,
                               originForUninitialized,
                               IMPLICIT_CONVERSION_ALLOWS_PRIVATE,
                               suppressErrors,
                               core::NameRef::noName()};
        auto dispatched = nonNilBlockType.type.dispatchCall(gs, innerArgs);
        for (auto &err : dispatched.main.errors) {
            gs._error(std::move(err));
        }

        if (typeIsNilable) {
            return Types::any(gs, dispatched.returnType, Types::nilClass());
        } else {
            return dispatched.returnType;
        }
    }

    static std::optional<int> getArityForBlock(const TypePtr &blockType) {
        if (auto *appliedType = cast_type<AppliedType>(blockType)) {
            return Types::getProcArity(*appliedType);
        }

        return std::nullopt;
    }

    static std::vector<ArgInfo::ArgFlags> argInfoByArity(std::optional<int> fixedArity) {
        std::vector<ArgInfo::ArgFlags> res;
        if (fixedArity) {
            for (int i = 0; i < *fixedArity; i++) {
                res.emplace_back();
            }
        } else {
            res.emplace_back().isRepeated = true;
        }
        return res;
    }

    static void showLocationOfArgDefn(const GlobalState &gs, ErrorBuilder &e, const TypePtr &blockType,
                                      DispatchComponent &dispatchComp) {
        if (!dispatchComp.method.exists()) {
            return;
        }

        const auto &methodArgs = dispatchComp.method.data(gs)->arguments;
        ENFORCE(!methodArgs.empty());
        const auto &bspec = methodArgs.back();
        ENFORCE(bspec.flags.isBlock);
        auto for_ = ErrorColors::format("block argument `{}` of method `{}`", bspec.argumentName(gs),
                                        dispatchComp.method.show(gs));
        e.addErrorSection(TypeAndOrigins::explainExpected(gs, blockType, bspec.loc, for_));
    }

    static void simulateCall(const GlobalState &gs, const TypeAndOrigins *receiver, const DispatchArgs &innerArgs,
                             shared_ptr<SendAndBlockLink> link, TypePtr passedInBlockType, Loc callLoc, Loc blockLoc,
                             DispatchResult &res) {
        auto dispatched = receiver->type.dispatchCall(gs, innerArgs);
        // We use isSubTypeUnderConstraint here with a TypeConstraint, so that we discover the correct generic bounds
        // as we do the subtyping check.
        auto &constr = dispatched.main.constr;
        auto &blockPreType = dispatched.main.blockPreType;
        // TODO(jez) How should this interact with highlight untyped?
        if (blockPreType && !Types::isSubTypeUnderConstraint(gs, *constr, passedInBlockType, blockPreType,
                                                             UntypedMode::AlwaysCompatible)) {
            auto nonNilableBlockType = Types::dropNil(gs, blockPreType);
            if (isa_type<ClassType>(passedInBlockType) &&
                cast_type_nonnull<ClassType>(passedInBlockType).symbol == Symbols::Proc() &&
                Types::isSubType(gs, nonNilableBlockType, passedInBlockType)) {
                // If a block of unknown arity is passed in, but the function was declared with a known arity,
                // raise an error in strict mode.
                // This could occur, for example, when using Method#to_proc, since we type it as returning a `Proc`.
                if (auto e = gs.beginError(blockLoc, errors::Infer::ProcArityUnknown)) {
                    e.setHeader("Cannot use a `{}` with unknown arity as a `{}`", "Proc", blockPreType.show(gs));
                    if (!dispatched.secondary) {
                        Magic_callWithBlock::showLocationOfArgDefn(gs, e, blockPreType, dispatched.main);
                    }
                }

                // Create a new proc of correct arity, with everything as untyped,
                // and then use this type instead of passedInBlockType in later subtype checks.
                // This allows the generic parameters to be instantiated with untyped rather than bottom.
                if (std::optional<int> procArity = Magic_callWithBlock::getArityForBlock(nonNilableBlockType)) {
                    vector<core::TypePtr> targs(*procArity + 1, core::Types::untypedUntracked());
                    auto procWithCorrectArity = core::Symbols::Proc(*procArity);
                    passedInBlockType = make_type<core::AppliedType>(procWithCorrectArity, move(targs));
                }
            } else if (auto e = gs.beginError(blockLoc, errors::Infer::MethodArgumentMismatch)) {
                e.setHeader("Expected `{}` but found `{}` for block argument", blockPreType.show(gs),
                            passedInBlockType.show(gs));
                if (!dispatched.secondary) {
                    Magic_callWithBlock::showLocationOfArgDefn(gs, e, blockPreType, dispatched.main);
                }
            }
        }

        {
            auto it = &dispatched;
            while (it != nullptr) {
                if (it->main.method.exists()) {
                    const auto &methodArgs = it->main.method.data(gs)->arguments;
                    ENFORCE(!methodArgs.empty());
                    const auto &bspec = methodArgs.back();
                    ENFORCE(bspec.flags.isBlock);

                    auto bspecType = bspec.type;
                    if (bspecType) {
                        // TODO(jez) How should this interact with highlight untyped?
                        // This subtype check is here to discover the correct generic bounds.
                        Types::isSubTypeUnderConstraint(gs, *constr, passedInBlockType, bspecType,
                                                        UntypedMode::AlwaysCompatible);
                    }
                }
                it = it->secondary.get();
            }
        }
        if (constr) {
            if (!constr->solve(gs)) {
                if (auto e = gs.beginError(callLoc, errors::Infer::GenericMethodConstraintUnsolved)) {
                    e.setHeader("Could not find valid instantiation of type parameters");
                }
                dispatched.returnType = core::Types::untypedUntracked();
            }

            if (!constr->isEmpty() && constr->isSolved()) {
                dispatched.returnType = Types::instantiate(gs, dispatched.returnType, *(constr));
            }
        }
        res = std::move(dispatched);
    }

public:
    vector<NameRef> dispatchesTo() const override {
        // This is in addition to the custom handling that we do in Substitute.cc for the dispatch
        // to the Symbol literal arg.
        return {core::Names::toProc()};
    }

    // NOTE: Since this method dispatches to one of its Symbol-literal arguments, it has special
    // handling in Substitute.cc to account for fast path updates.
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] is the block
        // args[3...] are the remaining arguments
        // equivalent to (args[0]).args[1](*args[3..], &args[2])

        if (args.args.size() < 3) {
            return;
        }

        if (!isa_type<NamedLiteralType>(args.args[1]->type)) {
            return;
        }
        auto lit = cast_type_nonnull<NamedLiteralType>(args.args[1]->type);
        if (!lit.derivesFrom(gs, Symbols::Symbol())) {
            return;
        }

        NameRef fn = lit.asName();
        auto &receiver = args.args[0];
        if (receiver->type.isUntyped()) {
            auto what = core::errors::Infer::errorClassForUntyped(gs, args.locs.file, receiver->type);
            if (auto e = gs.beginError(args.argLoc(0), what)) {
                e.setHeader("Call to method `{}` on `{}`", fn.show(gs), "T.untyped");
                TypeErrorDiagnostics::explainUntyped(gs, e, what, *receiver, args.originForUninitialized);
            }

            res.returnType = receiver->type;
            return;
        }

        if (!receiver->type.isFullyDefined()) {
            return;
        }

        if (isa_type<TypeVar>(args.args[2]->type)) {
            if (auto e = gs.beginError(args.argLoc(2), core::errors::Infer::GenericPassedAsBlock)) {
                e.setHeader("Passing generics as block arguments is not supported");
            }
            return;
        }

        uint16_t numPosArgs = args.numPosArgs - 3;
        InlinedVector<TypeAndOrigins, 2> sendArgStore;
        InlinedVector<LocOffsets, 2> sendArgLocs;
        for (int i = 3; i < args.args.size(); i++) {
            sendArgStore.emplace_back(*args.args[i]);
            sendArgLocs.emplace_back(args.locs.args[i]);
        }
        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        sendArgs.reserve(sendArgStore.size());
        for (auto &arg : sendArgStore) {
            sendArgs.emplace_back(&arg);
        }
        CallLocs sendLocs{args.locs.file, args.locs.call, args.locs.args[0], args.locs.fun, sendArgLocs};

        TypePtr finalBlockType =
            Magic_callWithBlock::typeToProc(gs, *args.args[2], args.locs.file, args.locs.call, args.locs.args[2],
                                            args.locs.fun, args.originForUninitialized, args.suppressErrors);
        std::optional<int> blockArity = Magic_callWithBlock::getArityForBlock(finalBlockType);
        auto link = make_shared<core::SendAndBlockLink>(fn, Magic_callWithBlock::argInfoByArity(blockArity), -1);
        res.main.constr = make_unique<TypeConstraint>();

        DispatchArgs innerArgs{fn,
                               sendLocs,
                               numPosArgs,
                               sendArgs,
                               receiver->type,
                               *receiver,
                               receiver->type,
                               link,
                               args.originForUninitialized,
                               args.isPrivateOk,
                               args.suppressErrors,
                               args.enclosingMethodForSuper};

        Magic_callWithBlock::simulateCall(gs, receiver, innerArgs, link, finalBlockType, args.argLoc(2), args.callLoc(),
                                          res);
    }
} Magic_callWithBlock;

class Magic_callWithSplatAndBlock : public IntrinsicMethod {
public:
    vector<NameRef> dispatchesTo() const override {
        // This is in addition to the custom handling that we do in Substitute.cc for the dispatch
        // to the Symbol literal arg.
        return {core::Names::toProc()};
    }

    // NOTE: Since this method dispatches to one of its Symbol-literal arguments, it has special
    // handling in Substitute.cc to account for fast path updates.
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] are the splat arguments
        // args[3] are the keyword arguments
        // args[4] is the block

        if (args.args.size() != 5) {
            return;
        }

        if (!isa_type<NamedLiteralType>(args.args[1]->type)) {
            return;
        }
        auto lit = cast_type_nonnull<NamedLiteralType>(args.args[1]->type);
        if (!lit.derivesFrom(gs, Symbols::Symbol())) {
            return;
        }

        NameRef fn = lit.asName();

        auto &receiver = args.args[0];
        if (receiver->type.isUntyped()) {
            auto what = core::errors::Infer::errorClassForUntyped(gs, args.locs.file, receiver->type);
            if (auto e = gs.beginError(args.argLoc(0), what)) {
                e.setHeader("Call to method `{}` on `{}`", fn.show(gs), "T.untyped");
                TypeErrorDiagnostics::explainUntyped(gs, e, what, *args.args[0], args.originForUninitialized);
            }

            res.returnType = receiver->type;
            return;
        }

        if (!receiver->type.isFullyDefined()) {
            return;
        }

        if (args.args[2]->type.isUntyped()) {
            auto what = core::errors::Infer::errorClassForUntyped(gs, args.locs.file, args.args[2]->type);
            if (auto e = gs.beginError(args.argLoc(2), what)) {
                e.setHeader("Call to method `{}` with `{}` splat arguments", fn.show(gs), "T.untyped");
                TypeErrorDiagnostics::explainUntyped(gs, e, what, *args.args[2], args.originForUninitialized);
            }

            res.returnType = args.args[2]->type;
            return;
        }
        auto *posTuple = cast_type<TupleType>(args.args[2]->type);
        if (posTuple == nullptr) {
            if (auto e = gs.beginError(args.argLoc(2), core::errors::Infer::UntypedSplat)) {
                e.setHeader("Splats are only supported where the size of the array is known statically");
            }
            return;
        }

        uint16_t numPosArgs = posTuple->elems.size();

        auto kwType = args.args[3]->type;
        auto *kwTuple = cast_type<TupleType>(kwType);
        if (kwTuple == nullptr && !kwType.isNilClass()) {
            if (auto e = gs.beginError(args.argLoc(2), core::errors::Infer::UntypedSplat)) {
                e.setHeader(
                    "Keyword args with splats are only supported where the shape of the hash is known statically");
            }
            return;
        }

        if (isa_type<TypeVar>(args.args[4]->type)) {
            if (auto e = gs.beginError(args.argLoc(4), core::errors::Infer::GenericPassedAsBlock)) {
                e.setHeader("Passing generics as block arguments is not supported");
            }
            return;
        }

        InlinedVector<TypeAndOrigins, 2> sendArgStore;
        InlinedVector<const TypeAndOrigins *, 2> sendArgs =
            Magic_callWithSplat::generateSendArgs(posTuple, kwTuple, sendArgStore, args.argLoc(2));
        InlinedVector<LocOffsets, 2> sendArgLocs(sendArgs.size(), args.locs.args[2]);
        CallLocs sendLocs{args.locs.file, args.locs.call, args.locs.args[0], args.locs.fun, sendArgLocs};

        TypePtr finalBlockType =
            Magic_callWithBlock::typeToProc(gs, *args.args[4], args.locs.file, args.locs.call, args.locs.args[4],
                                            args.locs.fun, args.originForUninitialized, args.suppressErrors);
        std::optional<int> blockArity = Magic_callWithBlock::getArityForBlock(finalBlockType);
        auto link = make_shared<core::SendAndBlockLink>(fn, Magic_callWithBlock::argInfoByArity(blockArity), -1);
        res.main.constr = make_unique<TypeConstraint>();

        DispatchArgs innerArgs{fn,
                               sendLocs,
                               numPosArgs,
                               sendArgs,
                               receiver->type,
                               *receiver,
                               receiver->type,
                               link,
                               args.originForUninitialized,
                               args.isPrivateOk,
                               args.suppressErrors,
                               args.enclosingMethodForSuper};

        Magic_callWithBlock::simulateCall(gs, receiver, innerArgs, link, finalBlockType, args.argLoc(4), args.callLoc(),
                                          res);
    }
} Magic_callWithSplatAndBlock;

class Magic_suggestUntypedConstantType : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        ENFORCE(args.args.size() == 1);
        auto ty = core::Types::widen(gs, args.args.front()->type);
        auto loc = args.argLoc(0);
        auto argLocExists = true;
        if (!loc.exists()) {
            // This might be because our argument was an EmptyTree (`begin; end`). Fall back to
            // using the loc of the whole send.
            argLocExists = false;
            loc = args.callLoc();
        }
        if (auto e = gs.beginError(loc, core::errors::Infer::UntypedConstantSuggestion)) {
            e.setHeader("Constants must have type annotations with `{}` when specifying `{}`", "T.let",
                        "# typed: strict");
            if ((gs.suggestUnsafe || !ty.isUntyped()) && loc.exists() && argLocExists) {
                // (skip the autocorrect if we had to fall back to using callLoc, because using that
                // will suggest something syntactically invalid like `T.let(U = begin; end, NilClass))`
                e.replaceWith(fmt::format("Initialize as `{}`", ty.show(gs)), loc, "T.let({}, {})",
                              loc.source(gs).value(), ty.show(gs));
            }
        }
        res.returnType = move(ty);
    }
} Magic_suggestUntypedConstantType;

class Magic_suggestUntypedFieldType : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        ENFORCE(args.args.size() == 4);
        res.returnType = core::Types::widen(gs, args.args[0]->type);

        if (args.suppressErrors) {
            return;
        }

        if (auto e = gs.beginError(args.callLoc(), core::errors::Infer::UntypedFieldSuggestion)) {
            const auto &fieldKindTy = cast_type_nonnull<NamedLiteralType>(args.args[1]->type);
            auto fieldKind = fieldKindTy.asName().show(gs);
            const auto &definingMethodNameTy = cast_type_nonnull<NamedLiteralType>(args.args[2]->type);
            auto definingMethodName = definingMethodNameTy.asName();
            const auto &fieldNameTy = cast_type_nonnull<NamedLiteralType>(args.args[3]->type);
            auto fieldName = fieldNameTy.asName().show(gs);

            auto suggestType = res.returnType;
            if (definingMethodName != core::Names::initialize() && definingMethodName != core::Names::staticInit() &&
                definingMethodName != core::Names::beforeAngles()) {
                suggestType = core::Types::any(gs, Types::nilClass(), suggestType);
            }
            e.setHeader("The {} variable `{}` must be declared using `{}` when specifying `{}`", fieldKind, fieldName,
                        "T.let", "# typed: strict");
            auto replaceLoc = args.argLoc(0);
            if ((gs.suggestUnsafe.has_value() || !suggestType.isUntyped()) && replaceLoc.exists()) {
                // Loc might not exist be because our argument was an EmptyTree (`begin; end`).
                // In that case we don't have an RHS we can easily wrap in something, so skip the autocorrect.
                auto title = fmt::format("Initialize as `{}`", suggestType.show(gs));

                if (replaceLoc.adjustLen(gs, -2, 1).source(gs) == "=") {
                    // Defensive; might be an ivar assignment from `attr_accessor` or `prop`, which
                    // we don't want an autocorrect for.
                    e.replaceWith(title, replaceLoc, "T.let({}, {})", replaceLoc.source(gs).value(),
                                  suggestType.show(gs));
                }
            }
        }
    }
} Magic_suggestUntypedFieldType;

class Magic_attachedClass : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }

        // args[0] is `<self>`

        auto selfTy = *args.args[0];
        auto mustExist = true;
        auto self = unwrapSymbol(gs, selfTy.type, mustExist);
        auto selfData = self.data(gs);

        auto attachedClass = selfData->findMember(gs, core::Names::Constants::AttachedClass());
        if (attachedClass.exists()) {
            res.returnType = make_type<MetaType>(make_type<SelfTypeParam>(attachedClass));
        } else if (self != core::Symbols::T_Private_Methods_DeclBuilder() && !args.suppressErrors) {
            if (auto e = gs.beginError(args.callLoc(), core::errors::Infer::AttachedClassOnInstance)) {
                auto hasAttachedClass = core::Names::declareHasAttachedClass().show(gs);
                if (selfData->isModule()) {
                    e.setHeader("`{}` must declare `{}` before module instance methods can use `{}`", self.show(gs),
                                hasAttachedClass, "T.attached_class");
                    // TODO(jez) Autocorrect to insert `has_attached_class!`
                } else if (selfData->isSingletonClass(gs)) {
                    // Combination of `isSingletonClass` and `<AttachedClass>` missing means
                    // this is the singleton class of a module.
                    ENFORCE(selfData->attachedClass(gs).data(gs)->isModule());
                    e.setHeader("`{}` cannot be used in singleton methods on modules, because modules cannot be "
                                "instantiated",
                                "T.attached_class");
                } else {
                    // Technically, this error message should also have something like "..., or
                    // instance methods on `::Class`", but that makes the error message wordy, and
                    // anyone who cares about that technicality likely knows what they're doing.
                    e.setHeader(
                        "`{}` may only be used in singleton methods on classes or instance methods on `{}` modules",
                        "T.attached_class", hasAttachedClass);
                    e.addErrorNote("Current context is `{}`, which is an instance class not a singleton class",
                                   self.show(gs));
                }
            }
            res.returnType = core::Types::untypedUntracked();
        }
    }
} Magic_attachedClass;

class Magic_checkAndAnd : public IntrinsicMethod {
public:
    // NOTE: Since this method dispatches to one of its Symbol-literal arguments, it has special
    // handling in Substitute.cc to account for fast path updates.
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // <Magic>.<check-and-and> is created when desugaring for the purpose of adding a more
        // specific error message when using `&&`. The normal error is something like "Method not
        // found on NilClass" which is common enough and confusing enough that it warrants getting
        // special wording (especially for first-time Sorbet users).
        //
        // args[0] is recv
        // args[1] is the method name
        // args[2] is the && tmp var
        // args[3...] are the args

        ENFORCE(args.args.size() >= 3, "Desugar should have created call to <check-and-and> with exactly 3 args");
        if (args.args.size() < 3) {
            return;
        }

        auto selfTy = *args.args[0];
        auto selfTyAndAnd = *args.args[2];

        if (!isa_type<NamedLiteralType>(args.args[1]->type)) {
            return;
        }
        auto lit = cast_type_nonnull<NamedLiteralType>(args.args[1]->type);
        if (!lit.derivesFrom(gs, Symbols::Symbol())) {
            return;
        }
        auto fun = lit.asName();

        uint16_t numPosArgs = args.numPosArgs - 3;

        InlinedVector<const TypeAndOrigins *, 2> sendArgStore;
        InlinedVector<LocOffsets, 2> sendArgLocs;
        for (int i = 3; i < args.args.size(); ++i) {
            sendArgStore.emplace_back(args.args[i]);
            sendArgLocs.emplace_back(args.locs.args[i]);
        }
        auto recvLoc = args.locs.args[0];
        CallLocs sendLocs{args.locs.file, args.locs.call, recvLoc, args.locs.fun, sendArgLocs};

        DispatchArgs innerArgs{
            fun,
            sendLocs,
            numPosArgs,
            sendArgStore,
            selfTy.type,
            selfTy,
            selfTy.type,
            args.block,
            args.originForUninitialized,
            args.isPrivateOk,
            args.suppressErrors,
            args.enclosingMethodForSuper,
        };
        auto dispatched = selfTy.type.dispatchCall(gs, innerArgs);

        auto multipleComponents = dispatched.secondary != nullptr;
        if (multipleComponents) {
            int unknownMethodOnNilClassErrors = 0;
            for (auto it = &dispatched; it != nullptr; it = it->secondary.get()) {
                for (auto &err : it->main.errors) {
                    if (err->what == core::errors::Infer::UnknownMethod && it->main.receiver.isNilClass()) {
                        unknownMethodOnNilClassErrors++;
                    }
                }
            }

            if (unknownMethodOnNilClassErrors == 1 && !core::Types::isSubType(gs, selfTy.type, selfTyAndAnd.type)) {
                DispatchArgs newInnerArgs{
                    fun,
                    sendLocs,
                    numPosArgs,
                    sendArgStore,
                    selfTyAndAnd.type,
                    selfTyAndAnd,
                    selfTyAndAnd.type,
                    args.block,
                    args.originForUninitialized,
                    // We already reported one visibility error, if relevant
                    /* isPrivateOk */ true,
                    args.suppressErrors,
                    args.enclosingMethodForSuper,
                };
                auto retried = selfTyAndAnd.type.dispatchCall(gs, newInnerArgs);

                auto foundErrorOnRetry = false;
                for (auto it = &retried; it != nullptr; it = it->secondary.get()) {
                    foundErrorOnRetry |= !it->main.errors.empty();
                }

                if (!foundErrorOnRetry) {
                    for (auto it = &dispatched; it != nullptr; it = it->secondary.get()) {
                        for (auto &err : it->main.errors) {
                            if (err->what == core::errors::Infer::UnknownMethod && it->main.receiver.isNilClass()) {
                                if (auto newErr = gs.beginError(err->loc, core::errors::Infer::CallAfterAndAnd)) {
                                    newErr.setHeader(
                                        "Call to method `{}` after `{}` assumes result type doesn't change",
                                        fun.show(gs), "&&");
                                    newErr.addErrorSection(selfTy.explainGot(gs, args.originForUninitialized));
                                    auto header = ErrorColors::format("Type had been narrowed to `{}` before `{}`:",
                                                                      selfTyAndAnd.type.show(gs), "&&");
                                    newErr.addErrorSection(ErrorSection(
                                        header, selfTyAndAnd.origins2Explanations(gs, args.originForUninitialized)));

                                    newErr.addErrorNote(
                                        "Sorbet never assumes that a method called twice returns the same "
                                        "result both times.\n    Either factor out a variable or use `{}`",
                                        "&.");

                                    auto funLoc = core::Loc(args.locs.file, args.locs.fun);
                                    if (selfTyAndAnd.origins.size() == 2 && funLoc.exists() && !funLoc.empty() &&
                                        funLoc.adjustLen(gs, -1, 1).source(gs) == ".") {
                                        // Our checkAndAnd desugarer only fires if the LHS and RHS have matching Send
                                        // nodes. In these cases, we know that there are two origins and that the first
                                        // origin is the call on the LHS.
                                        auto lhsLoc = selfTyAndAnd.origins[0];
                                        ENFORCE(lhsLoc.beginPos() < selfTyAndAnd.origins[1].beginPos());
                                        newErr.addAutocorrect(AutocorrectSuggestion{
                                            "Refactor to use `&.`",
                                            {
                                                AutocorrectSuggestion::Edit{
                                                    core::Loc(args.locs.file, lhsLoc.beginPos(), recvLoc.beginPos()),
                                                    "",
                                                },
                                                AutocorrectSuggestion::Edit{funLoc.adjustLen(gs, -1, 1), "&."},
                                            },
                                        });
                                    }

                                    err = newErr.build();
                                }
                            }
                        }
                    }
                }
            }
        }

        for (auto &err : res.main.errors) {
            dispatched.main.errors.emplace_back(std::move(err));
        }
        res = std::move(dispatched);
    }
} Magic_checkAndAnd;

class Magic_splat : public IntrinsicMethod {
public:
    vector<NameRef> dispatchesTo() const override {
        return {core::Names::toA()};
    }

    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        ENFORCE(args.args.size() == 1);

        auto &arg = args.args[0];
        InlinedVector<LocOffsets, 2> argLocs{args.locs.receiver};
        CallLocs locs{args.locs.file, args.locs.call, args.locs.call, args.locs.fun, argLocs};
        InlinedVector<const TypeAndOrigins *, 2> innerArgs;

        DispatchArgs dispatch{core::Names::toA(),
                              locs,
                              0,
                              innerArgs,
                              arg->type,
                              {arg->type, args.fullType.origins},
                              arg->type,
                              nullptr,
                              args.originForUninitialized,
                              IMPLICIT_CONVERSION_ALLOWS_PRIVATE,
                              args.suppressErrors,
                              args.enclosingMethodForSuper};
        auto dispatched = arg->type.dispatchCall(gs, dispatch);

        // The VM handles the case of an error when dispatching to_a, so the only
        // thing we need to ask is "did the call error?".
        if (!dispatched.main.errors.empty()) {
            // In case of an error, the splat is converted to an array with a single
            // element; be conservative in what we declare the element type to be.
            res.returnType = Types::arrayOfUntyped(Symbols::Magic_UntypedSource_splat());
        } else {
            res.returnType = dispatched.returnType;
        }
    };
} Magic_splat;

class Tuple_squareBrackets : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.empty() || args.args.size() > 2) {
            return;
        }

        auto *tuple = cast_type<TupleType>(args.thisType);
        ENFORCE(tuple);
        auto tupleSize = tuple->elems.size();

        auto argType = args.args.front()->type;
        if (!isa_type<IntegerLiteralType>(argType)) {
            // One day we might want to handle support for ranges,
            // but it's tricky because we don't have literal types for ranges.
            //
            // We might honestly have better luck by desugaring literal ranges to an equivalent
            // index+len call if supporting literal ranges isn't a good idea.
            return;
        }
        auto &lit = cast_type_nonnull<IntegerLiteralType>(argType);

        auto idx = lit.value;
        if (idx < 0) {
            idx = tupleSize + idx;
        }

        if (args.args.size() == 1) {
            if (idx >= tupleSize) {
                res.returnType = Types::nilClass();
            } else {
                res.returnType = tuple->elems[idx];
            }
            return;
        }

        ENFORCE(args.args.size() == 2);

        if (idx < 0 || idx > tupleSize) {
            res.returnType = Types::nilClass();
            return;
        }

        const auto &lengthType = args.args[1]->type;
        if (!isa_type<IntegerLiteralType>(lengthType)) {
            return;
        }
        const auto &lengthLit = cast_type_nonnull<IntegerLiteralType>(lengthType);
        auto length = lengthLit.value;
        if (length < 0) {
            res.returnType = Types::nilClass();
            return;
        }

        if (tupleSize < length || tupleSize < idx + length) {
            length = tupleSize - idx;
        }

        vector<TypePtr> newElems;
        for (long i = 0; i < length; i++) {
            newElems.emplace_back(tuple->elems[idx + i]);
        }
        res.returnType = make_type<TupleType>(move(newElems));
    }
} Tuple_squareBrackets;

class Tuple_last : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto *tuple = cast_type<TupleType>(args.thisType);
        ENFORCE(tuple);

        if (!args.args.empty()) {
            return;
        }
        if (tuple->elems.empty()) {
            res.returnType = Types::nilClass();
        } else {
            res.returnType = tuple->elems.back();
        }
    }
} Tuple_last;

class Tuple_first : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto *tuple = cast_type<TupleType>(args.thisType);
        ENFORCE(tuple);

        if (!args.args.empty()) {
            return;
        }
        if (tuple->elems.empty()) {
            res.returnType = Types::nilClass();
        } else {
            res.returnType = tuple->elems.front();
        }
    }
} Tuple_first;

class Tuple_minMax : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto *tuple = cast_type<TupleType>(args.thisType);
        ENFORCE(tuple);

        if (!args.args.empty()) {
            return;
        }
        if (tuple->elems.empty()) {
            res.returnType = Types::nilClass();
        } else {
            res.returnType = tuple->elementType(gs);
        }
    }
} Tuple_minMax;

class Tuple_sum : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto *tuple = cast_type<TupleType>(args.thisType);
        ENFORCE(tuple);

        if (!args.args.empty()) {
            return;
        }
        if (args.block != nullptr) {
            return;
        }
        if (tuple->elems.empty()) {
            res.returnType = Types::Integer();
        } else {
            res.returnType = tuple->elementType(gs);
        }
    }
} Tuple_sum;

class Tuple_sample : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto *tuple = cast_type<TupleType>(args.thisType);
        ENFORCE(tuple);

        if (args.args.size() > 1) {
            return;
        }
        if (args.block != nullptr) {
            return;
        }
        if (args.args.empty()) {
            if (tuple->elems.empty()) {
                res.returnType = Types::nilClass();
            } else {
                res.returnType = tuple->elementType(gs);
            }
        } else {
            if (tuple->elems.empty()) {
                res.returnType = make_type<TupleType>(vector<TypePtr>{});
            } else {
                res.returnType = Types::arrayOf(gs, tuple->elementType(gs));
            }
        }
    }
} Tuple_sample;

class Tuple_to_a : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        res.returnType = args.selfType;
    }
} Tuple_to_a;

class Tuple_concat : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        vector<TypePtr> elems;
        auto *tuple = cast_type<TupleType>(args.thisType);
        ENFORCE(tuple);
        elems = tuple->elems;
        for (auto elem : args.args) {
            if (auto *tuple = cast_type<TupleType>(elem->type)) {
                elems.insert(elems.end(), tuple->elems.begin(), tuple->elems.end());
            } else {
                return;
            }
        }
        res.returnType = make_type<TupleType>(std::move(elems));
    }
} Tuple_concat;

optional<Loc> locOfValueForKey(const GlobalState &gs, const Loc origin, const NameRef key, const TypePtr expectedType) {
    if (!isa_type<ClassType>(expectedType)) {
        return nullopt;
    }
    auto ct = cast_type_nonnull<ClassType>(expectedType);

    // Unlike with normal `T.let` autocorrects, we don't have location information for "the value
    // for a specific key". To make up for this, we hard-code the most common pinning errors by
    // scanning the source directly.

    const char *valueStr;
    if (ct.symbol == core::Symbols::NilClass()) {
        valueStr = "nil";
    } else if (ct.symbol == core::Symbols::TrueClass()) {
        valueStr = "true";
    } else if (ct.symbol == core::Symbols::FalseClass()) {
        valueStr = "false";
    } else {
        return nullopt;
    }

    auto source = origin.source(gs);
    if (!source.has_value()) {
        return nullopt;
    }

    auto keySymbol = fmt::format("{}:", key.shortName(gs));
    auto keyStart = source->find(keySymbol);
    if (keyStart == string::npos) {
        return nullopt;
    }

    // TODO(jez) Use Loc::adjust here
    uint32_t valueBegin = origin.beginPos() + keyStart + keySymbol.size() + char_traits<char>::length(" ");
    uint32_t valueEnd = valueBegin + char_traits<char>::length(valueStr);
    if (valueEnd <= origin.file().data(gs).source().size()) {
        auto loc = Loc{origin.file(), valueBegin, valueEnd};
        if (loc.exists() && loc.source(gs).value() == valueStr) {
            return loc;
        }
    }

    return nullopt;
}

class Shape_squareBracketsEq : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto &shape = cast_type_nonnull<ShapeType>(args.thisType);

        if (args.args.size() != 2) {
            // Skip over cases for which arg matching should report errors
            return;
        }

        auto &arg = args.args.front()->type;
        if (!isa_type<NamedLiteralType>(arg) && !isa_type<IntegerLiteralType>(arg) &&
            !isa_type<FloatLiteralType>(arg)) {
            return;
        }

        if (auto idx = shape.indexForKey(arg)) {
            auto valueType = shape.values[*idx];
            auto expectedType = valueType;
            auto actualType = *args.args[1];
            // This check (with the dropLiteral's) mimics what we do for pinning errors in environment.cc
            // TODO(jez) How should this interact with highlight untyped?
            if (!Types::isSubType(gs, Types::dropLiteral(gs, actualType.type), Types::dropLiteral(gs, expectedType))) {
                auto argLoc = args.argLoc(1);

                if (auto e = gs.beginError(argLoc, errors::Infer::MethodArgumentMismatch)) {
                    e.setHeader("Expected `{}` but found `{}` for key `{}`", expectedType.show(gs),
                                actualType.type.show(gs), shape.keys[*idx].show(gs));
                    e.addErrorSection(
                        ErrorSection("Shape originates from here:",
                                     args.fullType.origins2Explanations(gs, args.originForUninitialized)));
                    e.addErrorSection(actualType.explainGot(gs, args.originForUninitialized));

                    if (args.fullType.origins.size() == 1 && isa_type<NamedLiteralType>(arg)) {
                        auto argLit = cast_type_nonnull<NamedLiteralType>(arg);
                        if (argLit.literalKind == NamedLiteralType::LiteralTypeKind::Symbol) {
                            auto key = argLit.asName();
                            auto loc = locOfValueForKey(gs, args.fullType.origins[0], key, expectedType);

                            if (loc.has_value() && loc->exists()) {
                                e.replaceWith("Initialize with `T.let`", *loc, "T.let({}, {})", loc->source(gs).value(),
                                              Types::any(gs, expectedType, actualType.type).show(gs));
                            }
                        }
                    }
                }
            }

            // Returning here without setting res.resultType will cause dispatchCall to fall back to
            // Hash#[]=, which will have the effect of checking the arg types.
            //
            // TODO(jez) Right now ShapeType::underlying always returns T::Hash[T.untyped, T.untyped]
            // so it doesn't matter whether we return or not.
            return;
        } else {
            // Key not found. To preserve legacy compatibility, allow any arguments here.
            // I would love to remove this one day, but we'll have to figure out a way to migrate
            // people's codebases to it.
            //
            // TODO(jez) This could be another "if you're in `typed: strict` you need typed shapes"
            res.returnType = Types::untyped(Symbols::Magic_UntypedSource_shapeSquareBracketsEq());
        }
    }
} Shape_squareBracketsEq;

class Shape_merge : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto *shape = cast_type<ShapeType>(args.thisType);
        ENFORCE(shape);

        if (args.args.empty() || args.block != nullptr) {
            return;
        }

        // detect a kwsplat argument, or single positional hash argument
        auto nonPosArgs = (args.args.size() - args.numPosArgs);
        auto numKwargs = nonPosArgs & ~0x1;
        bool hasKwsplat = nonPosArgs & 0x1;
        const ShapeType *kwsplat = nullptr;
        if (hasKwsplat || (numKwargs == 0 && args.args.size() == 1)) {
            kwsplat = cast_type<ShapeType>(args.args.back()->type);
            if (kwsplat == nullptr) {
                return;
            }
        }

        // Deliberately copy the keys and values, since we may be adding entries.
        auto copyTypePtr = make_type<ShapeType>(shape->keys, shape->values);
        auto *copy = cast_type<ShapeType>(copyTypePtr);
        ENFORCE(copy != nullptr);
        auto addShapeEntry = [&copy](const TypePtr &keyType, const TypePtr &value) {
            if (auto optind = copy->indexForKey(keyType)) {
                copy->values[*optind] = value;
            } else {
                copy->keys.emplace_back(keyType);
                copy->values.emplace_back(value);
            }
        };

        // inlined keyword arguments first
        for (auto i = 0; i < numKwargs; i += 2) {
            auto &keyType = args.args[i]->type;
            if (!isa_type<NamedLiteralType>(keyType)) {
                return;
            }

            auto key = cast_type_nonnull<NamedLiteralType>(keyType);
            if (key.literalKind != NamedLiteralType::LiteralTypeKind::Symbol) {
                return;
            }

            addShapeEntry(keyType, args.args[i + 1]->type);
        }

        // then kwsplat
        if (kwsplat != nullptr) {
            for (auto &keyType : kwsplat->keys) {
                if (!isa_type<NamedLiteralType>(keyType) && !isa_type<IntegerLiteralType>(keyType) &&
                    !isa_type<FloatLiteralType>(keyType)) {
                    return;
                }
                addShapeEntry(keyType, kwsplat->values[&keyType - &kwsplat->keys.front()]);
            }
        }

        res.returnType = std::move(copyTypePtr);
    }
} Shape_merge;

class Shape_to_hash : public IntrinsicMethod {
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        res.returnType = args.selfType;
    }
} Shape_to_hash;

// `<Magic>.<to-hash-dup>(x)` and `<Magic>.<to-hash-nodup>(x) both behave like `x.to_hash`
class Magic_toHash : public IntrinsicMethod {
public:
    vector<NameRef> dispatchesTo() const override {
        return {core::Names::toHash()};
    }

    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        ENFORCE(args.args.size() == 1);

        auto &arg = args.args[0];
        InlinedVector<LocOffsets, 2> argLocs;
        CallLocs locs{args.locs.file, args.locs.call, args.locs.call, args.locs.fun, argLocs};
        InlinedVector<const TypeAndOrigins *, 2> innerArgs;

        DispatchArgs dispatch{core::Names::toHash(),
                              locs,
                              0,
                              innerArgs,
                              arg->type,
                              {arg->type, args.fullType.origins},
                              arg->type,
                              nullptr,
                              args.originForUninitialized,
                              IMPLICIT_CONVERSION_ALLOWS_PRIVATE,
                              args.suppressErrors,
                              args.enclosingMethodForSuper};
        res = arg->type.dispatchCall(gs, dispatch);
    }

} Magic_toHash;

// Interpret this `<Magic>.<merge-hash>(a, b)` as `a.merge!(b)`.
//
// NOTE: this will not update the type of `a` as `merge!` would in ruby. However, we only ever emit calls to this
// intrinsic that look like `a = <Magic>.<merge-hash>(a, b)`, so the fact that `merge!` won't update the type of the
// receiver isn't a problem here.
class Magic_mergeHash : public IntrinsicMethod {
    vector<NameRef> dispatchesTo() const override {
        return {core::Names::merge()};
    }

    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        ENFORCE(args.args.size() == 2);

        auto accType = args.args.front()->type;

        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        InlinedVector<LocOffsets, 2> sendArgLocs;

        sendArgs.emplace_back(args.args[1]);
        sendArgLocs.emplace_back(args.locs.args[1]);

        CallLocs sendLocs{args.locs.file, args.locs.call, args.locs.receiver, args.locs.fun, sendArgLocs};

        // emulate a call to `resType#merge`
        DispatchArgs mergeArgs{core::Names::merge(),
                               sendLocs,
                               1,
                               sendArgs,
                               accType,
                               {accType, args.args[0]->origins},
                               accType,
                               nullptr,
                               args.originForUninitialized,
                               args.isPrivateOk,
                               args.suppressErrors,
                               args.enclosingMethodForSuper};

        res = accType.dispatchCall(gs, mergeArgs);
    }
} Magic_mergeHash;

// Interpret this `<Magic>.<merge-hash-values>(a, k, v, ...)` as `a.merge!({k => v, ...})`
//
// See the note on Magic_mergeHash for why this doesn't dispatch to `merge!`.
class Magic_mergeHashValues : public IntrinsicMethod {
    vector<NameRef> dispatchesTo() const override {
        return {core::Names::merge()};
    }

    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // Argument format is
        // 0 - the hash to merge values into
        // 1 - the first key
        // 2 - the first value
        // ...
        ENFORCE(args.args.size() % 2 == 1);

        auto accType = args.args.front()->type;

        TypePtr argType = nullptr;

        vector<TypePtr> keys;
        vector<TypePtr> values;
        for (auto it = args.args.begin() + 1; it != args.args.end();) {
            auto *key = *it++;

            // Guard shape construction on keys being valid, falling back on T::Hash[T.untyped, T.untyped] if it's
            // invalid.
            if (!isa_type<NamedLiteralType>(key->type)) {
                argType = Types::hashOfUntyped(Symbols::Magic_UntypedSource_mergeHashValues());
                break;
            }

            auto *value = *it++;
            keys.emplace_back(key->type);
            values.emplace_back(value->type);
        }

        if (argType == nullptr) {
            argType = make_type<ShapeType>(std::move(keys), std::move(values));
        }

        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        InlinedVector<LocOffsets, 2> sendArgLocs;

        TypeAndOrigins argument{argType, InlinedVector<Loc, 1>{}};
        sendArgs.emplace_back(&argument);

        auto hashLoc = args.locs.args[1].join(args.locs.args.back());
        sendArgLocs.emplace_back(hashLoc);

        CallLocs sendLocs{args.locs.file, args.locs.call, args.locs.receiver, args.locs.fun, sendArgLocs};

        // emulate a call to `resType#merge` with a shape argument
        DispatchArgs mergeArgs{core::Names::merge(),
                               sendLocs,
                               1,
                               sendArgs,
                               accType,
                               {accType, args.args[0]->origins},
                               accType,
                               nullptr,
                               args.originForUninitialized,
                               args.isPrivateOk,
                               args.suppressErrors,
                               args.enclosingMethodForSuper};

        res = accType.dispatchCall(gs, mergeArgs);
    }
} Magic_mergeHashValues;

void digImplementation(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res, NameRef methodToDigWith) {
    if (args.args.size() == 0 || args.numPosArgs != args.args.size()) {
        // A type error was already reported for arg mismatch
        return;
    }

    auto baseCaseArgLocs = InlinedVector<LocOffsets, 2>{};
    baseCaseArgLocs.emplace_back(args.locs.args[0]);
    auto baseCaseLocs = CallLocs{
        args.locs.file,                         // file
        args.locs.call.copyWithZeroLength(),    // call
        args.locs.args[0].copyWithZeroLength(), // receiver
        args.locs.args[0].copyWithZeroLength(), // fun
        baseCaseArgLocs,                        // args
    };
    auto baseCaseArgTypes = InlinedVector<const TypeAndOrigins *, 2>{};
    baseCaseArgTypes.emplace_back(args.args[0]);
    auto baseCaseArgs = DispatchArgs{
        methodToDigWith,  baseCaseLocs,        1, /* numPosArgs */
        baseCaseArgTypes, args.selfType,       {args.selfType, args.fullType.origins},
        args.selfType,    args.block,          args.originForUninitialized,
        args.isPrivateOk, args.suppressErrors, args.enclosingMethodForSuper,
    };

    auto dispatched = args.selfType.dispatchCall(gs, baseCaseArgs);
    for (auto &err : dispatched.main.errors) {
        res.main.errors.emplace_back(std::move(err));
    }

    if (args.args.size() == 1) {
        res.returnType = move(dispatched.returnType);
        return;
    }

    auto newSelfType = Types::dropNil(gs, dispatched.returnType);

    if (newSelfType.isBottom()) {
        // The result was `nil` (or maybe `T.nilable(T.noreturn)` == `nil`, or maybe just plain `T.noreturn`)
        // In all these cases, Ruby's `dig` simply stops with `nil`, but in our case, we think it's
        // more helpful to let the user know that the extra args are effectively dead calls to `dig`.
        if (auto e = gs.beginError(args.argLoc(1), core::errors::Infer::DigExtraArgs)) {
            e.setHeader("Unused arguments to `{}`", "dig");
            e.addErrorLine(args.argLoc(0),
                           "The remaining `{}` arguments will not be used because the previous argument "
                           "dug a value that is always `{}` here",
                           "dig", "nil");
            auto endLastGood = args.locs.args[0].endPos();
            auto endAll = args.locs.args.back().endPos();
            auto replaceLoc = core::Loc(args.locs.file, endLastGood, endAll);
            if (endLastGood <= endAll && replaceLoc.exists()) {
                e.replaceWith("Delete extra args", replaceLoc, "");
            }
        }
        res.returnType = move(dispatched.returnType);
        return;
    }

    // ---- Recursive case ----

    auto digArgLocs = InlinedVector<LocOffsets, 2>{};
    for (int i = 1; i < args.locs.args.size(); i++) {
        digArgLocs.emplace_back(args.locs.args[i]);
    }
    auto digLocs = CallLocs{
        args.locs.file,                         // file
        args.locs.args[1],                      // call
        args.locs.receiver,                     // receiver
        args.locs.args[1].copyWithZeroLength(), // fun
        digArgLocs,                             // args
    };
    uint16_t newNumPosArgs = args.numPosArgs - 1;
    auto digArgTypes = InlinedVector<const TypeAndOrigins *, 2>{};
    for (int i = 1; i < args.args.size(); i++) {
        digArgTypes.emplace_back(args.args[i]);
    }
    auto fullTypeOrigins = InlinedVector<Loc, 1>{};
    for (const auto &origin : args.fullType.origins) {
        fullTypeOrigins.emplace_back(origin);
    }
    fullTypeOrigins.emplace_back(args.argLoc(0));
    auto newFullType = TypeAndOrigins{newSelfType, fullTypeOrigins};
    DispatchArgs digArgs{
        Names::dig(),
        digLocs,
        newNumPosArgs,
        digArgTypes,
        newSelfType,
        newFullType,
        newSelfType,
        args.block,
        args.originForUninitialized,
        false, /* isPrivateOk */
        args.suppressErrors,
        args.enclosingMethodForSuper,
    };

    auto recursiveDispatch = newSelfType.dispatchCall(gs, digArgs);

    for (auto it = &recursiveDispatch; it != nullptr; it = it->secondary.get()) {
        for (auto &err : it->main.errors) {
            res.main.errors.emplace_back(std::move(err));
        }
    }

    res.returnType = move(recursiveDispatch.returnType);
}

class Hash_dig : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        digImplementation(gs, args, res, Names::squareBrackets());
    }
} Hash_dig;

class Array_dig : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        digImplementation(gs, args, res, Names::at());
    }
} Array_dig;

class Array_flatten : public IntrinsicMethod {
    // If the element type supports the #to_ary method, then Ruby will implicitly call it when flattening. So here we
    // dispatch #to_ary and recurse further down the result if it succeeds, otherwise we just return the type.
    static TypePtr typeToAry(const GlobalState &gs, const DispatchArgs &args, const TypePtr &type,
                             const int64_t depth) {
        if (type.isUntyped()) {
            return type;
        }

        NameRef toAry = core::Names::toAry();

        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        InlinedVector<LocOffsets, 2> sendArgLocs;
        CallLocs sendLocs{args.locs.file, args.locs.call, args.locs.receiver, args.locs.fun, sendArgLocs};

        DispatchArgs innerArgs{toAry,
                               sendLocs,
                               0,
                               sendArgs,
                               type,
                               {type, args.fullType.origins},
                               type,
                               nullptr,
                               args.originForUninitialized,
                               IMPLICIT_CONVERSION_ALLOWS_PRIVATE,
                               args.suppressErrors,
                               args.enclosingMethodForSuper};

        auto dispatched = type.dispatchCall(gs, innerArgs);
        if (dispatched.main.errors.empty()) {
            if (dispatched.returnType.isNilClass()) {
                // According to the Ruby spec, it is valid for `to_ary` to return `nil`
                // which will stop `flatten` descending further.
                //
                // So, when we find a `NilClass` return type from `to_ary`, the type
                // we have is the one we are looking for.
                return type;
            } else {
                return recursivelyFlattenArrays(gs, args, move(dispatched.returnType), depth);
            }
        }

        return type;
    }

    // Flattens a (nested) array all way down to its (inner) element type, stopping if we hit the depth limit first.
    static TypePtr recursivelyFlattenArrays(const GlobalState &gs, const DispatchArgs &args, const TypePtr &type,
                                            const int64_t depth) {
        ENFORCE(type != nullptr);

        if (depth == 0) {
            return type;
        }

        TypePtr result;
        typecase(
            type,

            // This only shows up because t->elementType(gs) for tuples returns an OrType of all its elements.
            // So to properly handle nested tuples, we have to descend into the OrType's.
            [&](const OrType &o) {
                result = Types::any(gs, recursivelyFlattenArrays(gs, args, o.left, depth),
                                    recursivelyFlattenArrays(gs, args, o.right, depth));
            },

            [&](const ClassType &c) { result = typeToAry(gs, args, type, depth); },

            [&](const AppliedType &a) {
                if (a.klass == Symbols::Array()) {
                    ENFORCE(a.targs.size() == 1);
                    result = recursivelyFlattenArrays(gs, args, a.targs.front(), depth - 1);
                    return;
                }

                result = typeToAry(gs, args, type, depth);
            },

            [&](const TupleType &t) { result = recursivelyFlattenArrays(gs, args, t.elementType(gs), depth - 1); },

            [&](const TypePtr &t) { result = std::move(type); });
        return result;
    }

public:
    vector<NameRef> dispatchesTo() const override {
        return {core::Names::toAry()};
    }

    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // Unwrap the array one time to get the element type (we'll rewrap it down at the bottom)
        TypePtr element;
        auto *ap = cast_type<AppliedType>(args.thisType);
        ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(gs)->derivesFrom(gs, Symbols::Array()));
        ENFORCE(!ap->targs.empty());
        element = ap->targs.front();

        int64_t MAX_DEPTH = 100;
        int64_t depth;
        if (args.args.size() == 1) {
            auto argTyp = args.args[0]->type;
            ENFORCE(args.locs.args.size() == 1, "Mismatch between args.size() and args.locs.args.size(): {}",
                    args.locs.args.size());

            if (!isa_type<IntegerLiteralType>(argTyp)) {
                if (auto e = gs.beginError(args.argLoc(0), core::errors::Infer::ExpectedLiteralType)) {
                    e.setHeader("You must pass an Integer literal to specify a depth with Array#flatten");
                }
                return;
            }

            auto &lt = cast_type_nonnull<IntegerLiteralType>(argTyp);

            if (lt.value >= 0) {
                depth = lt.value;
            } else {
                // Negative values behave like no depth was given
                depth = MAX_DEPTH;
            }
        } else if (args.args.size() == 0) {
            depth = MAX_DEPTH;
        } else {
            // If our arity is off, then calls.cc will report an error due to mismatch with the RBI elsewhere, so we
            // don't need to do anything special here
            return;
        }

        res.returnType = Types::arrayOf(gs, recursivelyFlattenArrays(gs, args, element, depth));
    }
} Array_flatten;

class Array_product : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        vector<TypePtr> unwrappedElems;
        unwrappedElems.reserve(args.args.size() + 1);

        auto *ap = cast_type<AppliedType>(args.thisType);
        ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(gs)->derivesFrom(gs, Symbols::Array()));
        ENFORCE(!ap->targs.empty());
        unwrappedElems.emplace_back(ap->targs.front());

        for (auto arg : args.args) {
            auto argTyp = arg->type;
            if (auto *ap = cast_type<AppliedType>(argTyp)) {
                ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(gs)->derivesFrom(gs, Symbols::Array()));
                ENFORCE(!ap->targs.empty());
                unwrappedElems.emplace_back(ap->targs.front());
            } else if (auto *tuple = cast_type<TupleType>(argTyp)) {
                unwrappedElems.emplace_back(tuple->elementType(gs));
            } else {
                // Arg type didn't match; we already reported an error for the arg type; just return untyped to recover.
                res.returnType = Types::untypedUntracked();
                return;
            }
        }

        res.returnType = Types::arrayOf(gs, make_type<TupleType>(move(unwrappedElems)));
    }
} Array_product;

class Array_compact : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        TypePtr element;

        auto *ap = cast_type<AppliedType>(args.thisType);
        ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(gs)->derivesFrom(gs, Symbols::Array()));
        ENFORCE(!ap->targs.empty());
        element = ap->targs.front();

        auto ret = Types::dropNil(gs, element);
        res.returnType = Types::arrayOf(gs, ret);
    }
} Array_compact;

class Array_zip : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        vector<TypePtr> unwrappedElems;
        unwrappedElems.reserve(args.args.size() + 1);

        auto *ap = cast_type<AppliedType>(args.thisType);
        ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(gs)->derivesFrom(gs, Symbols::Array()));
        ENFORCE(!ap->targs.empty());
        unwrappedElems.emplace_back(ap->targs.front());

        for (auto arg : args.args) {
            auto argTyp = arg->type;
            if (auto *ap = cast_type<AppliedType>(argTyp)) {
                ENFORCE(ap->klass == Symbols::Enumerable() ||
                        ap->klass.data(gs)->derivesFrom(gs, Symbols::Enumerable()));
                ENFORCE(!ap->targs.empty());
                unwrappedElems.emplace_back(Types::any(gs, ap->targs.front(), Types::nilClass()));
            } else if (auto *tuple = cast_type<TupleType>(argTyp)) {
                unwrappedElems.emplace_back(Types::any(gs, tuple->elementType(gs), Types::nilClass()));
            } else {
                // Arg type didn't match; we already reported an error for the arg type; just return untyped to recover.
                res.returnType = Types::untypedUntracked();
                return;
            }
        }

        res.returnType = Types::arrayOf(gs, make_type<TupleType>(move(unwrappedElems)));
    }
} Array_zip;

class Array_transpose : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto *ap = cast_type<AppliedType>(args.thisType);
        ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(gs)->derivesFrom(gs, Symbols::Array()));
        ENFORCE(!ap->targs.empty());
        auto &elementType = ap->targs.front();

        auto *tuple = cast_type<TupleType>(elementType);
        if (tuple == nullptr) {
            return;
        }

        // The transpose of an array of tuples is a tuple of arrays.
        vector<TypePtr> transposed;
        transposed.reserve(tuple->elems.size());

        for (auto &elem : tuple->elems) {
            transposed.emplace_back(Types::arrayOf(gs, elem));
        }

        res.returnType = make_type<TupleType>(move(transposed));
    }
} Array_transpose;

class Symbol_eqeq : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }

        // NOTE:
        // If you update this, please update error-reference to mention which types this check applies to
        // TODO(jez) How should this interact with highlight untyped?
        auto isOnlySymbol =
            Types::isSubType(gs, args.fullType.type, Types::any(gs, Types::nilClass(), Types::Symbol()));
        if (isOnlySymbol && Types::all(gs, args.fullType.type, args.args[0]->type).isBottom()) {
            if (auto e = gs.beginError(args.errLoc(), errors::Infer::NonOverlappingEqual)) {
                e.setHeader("Comparison between `{}` and `{}` is always false", args.fullType.type.show(gs),
                            args.args[0]->type.show(gs));
                e.addErrorSection(args.fullType.explainGot(gs, args.originForUninitialized));
                e.addErrorSection(args.args[0]->explainGot(gs, args.originForUninitialized));
                if (args.args[0]->type == Types::String() && args.argLoc(0).exists()) {
                    e.replaceWith("Convert arg to Symbol", args.argLoc(0).copyEndWithZeroLength(), ".to_sym");
                }
            }
        }
    }
} Symbol_eqeq;

class String_eqeq : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }

        // TODO(jez) How should this interact with highlight untyped?
        auto isOnlyString =
            Types::isSubType(gs, args.fullType.type, Types::any(gs, Types::nilClass(), Types::String()));

        // NOTE:
        // If you update this, please update error-reference to mention which types this check applies to
        if (isOnlyString &&
            // This extra `isSubType` is here (not in Symbol_eqeq) because of how implicit String#==
            // allows implicit conversions with `to_str`. In essence, this check implements the
            // assumption that `Symbol` is final and thus no subclass can implement `to_str` (we
            // could also implement the actual final logic for arbitrary classes, but have opted not
            // to because it would likely be a cause for surprise).
            Types::isSubType(gs, args.args[0]->type, Types::any(gs, Types::nilClass(), Types::Symbol())) &&
            Types::all(gs, args.fullType.type, args.args[0]->type).isBottom()) {
            if (auto e = gs.beginError(args.errLoc(), errors::Infer::NonOverlappingEqual)) {
                e.setHeader("Comparison between `{}` and `{}` is always false", args.fullType.type.show(gs),
                            args.args[0]->type.show(gs));
                e.addErrorSection(args.fullType.explainGot(gs, args.originForUninitialized));
                e.addErrorSection(args.args[0]->explainGot(gs, args.originForUninitialized));
                if (args.args[0]->type == Types::Symbol() && args.receiverLoc().exists()) {
                    e.replaceWith("Convert arg to Symbol", args.receiverLoc().copyEndWithZeroLength(), ".to_sym");
                }
            }
        }
    }
} String_eqeq;

class Kernel_proc : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.block == nullptr) {
            return;
        }

        std::optional<int> numberOfPositionalBlockParams = args.block->fixedArity();
        if (!numberOfPositionalBlockParams || *numberOfPositionalBlockParams > core::Symbols::MAX_PROC_ARITY) {
            res.returnType = core::Types::procClass();
            return;
        }

        if (args.name == core::Names::lambda()) {
            // Handled by the Kernel#lambda overload, generated by generate_procs
            // TODO(jez) Can we expand to proc?
            return;
        }

        auto untypedWithBlame = core::Types::untyped(Symbols::Magic_UntypedSource_proc());
        vector<core::TypePtr> targs(*numberOfPositionalBlockParams + 1, untypedWithBlame);
        auto procClass = core::Symbols::Proc(*numberOfPositionalBlockParams);
        res.returnType = make_type<core::AppliedType>(procClass, move(targs));
    }
} Kernel_proc;

class Kernel_raise : public IntrinsicMethod {
public:
    vector<NameRef> dispatchesTo() const override {
        // Technically only dispatches to `new` but we manually flatten the chain to avoid having to
        // compute the transitive closure of dispatchesTo.
        return {core::Names::new_(), core::Names::initialize()};
    }

    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() < 1) {
            return;
        }
        auto classArg = args.args[0];

        // The isUntyped check is part performance optimization and part "correctness":
        // - no need to run another dispatch if it's just going to be untyped, but also...
        // - arguably none of this intrinsic should run if we don't know that arg0 isn't a Class
        //
        // Technically speaking, the Ruby VM actually implements this by way of calling
        // arg0.exception. It just so happens that Exception.exception is defined to forward to
        // `.new` (https://ruby-doc.org/core-2.7.2/Exception.html#method-c-exception) but anything
        // that defines a method called `exception` could be called. We don't implement that here,
        // because implicit conversions are hard in the presence of subtyping. Instead, we restrict
        // the check to only subclasses of `Exception`, not arbitrary classes.
        auto classOfException = make_type<ClassType>(Symbols::Exception().data(gs)->lookupSingletonClass(gs));
        if (!classArg->type.isUntyped() && !Types::isSubType(gs, classArg->type, classOfException)) {
            return;
        }

        uint16_t newNumPosArgs = args.args.size() >= 2 ? 1 : 0;
        auto newSendArgLocs = InlinedVector<LocOffsets, 2>();
        if (newNumPosArgs > 0) {
            newSendArgLocs.emplace_back(args.locs.args[1]);
        }
        auto newCallLocs = CallLocs{
            args.locs.file,
            /* callLoc */ args.locs.args[0].join(args.locs.args[newNumPosArgs]),
            /* receiverLoc */ args.locs.args[0],
            /* funLoc */ args.locs.args[0].copyEndWithZeroLength(),
            newSendArgLocs,
        };

        auto newSendArgs = InlinedVector<const TypeAndOrigins *, 2>();
        if (newNumPosArgs) {
            newSendArgs.emplace_back(args.args[1]);
        }

        DispatchArgs newArgs{
            Names::new_(),
            newCallLocs,
            newNumPosArgs,
            newSendArgs,
            classArg->type,
            *classArg,
            classArg->type,
            /* block */ nullptr,
            args.originForUninitialized,
            IMPLICIT_CONVERSION_ALLOWS_PRIVATE,
            args.suppressErrors,
            args.enclosingMethodForSuper,
        };
        auto dispatched = classArg->type.dispatchCall(gs, newArgs);

        for (auto it = &dispatched; it != nullptr; it = it->secondary.get()) {
            for (auto &err : it->main.errors) {
                res.main.errors.emplace_back(std::move(err));
            }
        }
    }
} Kernel_raise;

class Enumerable_toH : public IntrinsicMethod {
public:
    vector<NameRef> dispatchesTo() const override {
        return {core::Names::enumerableToH()};
    }

    // Forward Enumerable.to_h to RubyType.enumerable_to_h[self]
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        // Exit early when this is the case that's handled by the enumerable.rbi sig
        // The result type will already have been populated by dispatchCall using the sig.
        if (args.args.empty() && args.block != nullptr) {
            return;
        }

        auto hash = make_type<ClassType>(core::Symbols::Sorbet_Private_Static().data(gs)->lookupSingletonClass(gs));
        InlinedVector<LocOffsets, 2> argLocs{args.locs.receiver};
        CallLocs locs{
            args.locs.file, args.locs.call, args.locs.call, args.locs.fun, argLocs,
        };
        TypeAndOrigins myType{args.selfType, args.receiverLoc()};
        InlinedVector<const TypeAndOrigins *, 2> innerArgs{&myType};

        DispatchArgs dispatch{core::Names::enumerableToH(),
                              locs,
                              1,
                              innerArgs,
                              hash,
                              {hash, args.fullType.origins},
                              hash,
                              nullptr,
                              args.originForUninitialized,
                              args.isPrivateOk,
                              args.suppressErrors,
                              args.enclosingMethodForSuper};
        auto dispatched = hash.dispatchCall(gs, dispatch);
        for (auto &err : dispatched.main.errors) {
            res.main.errors.emplace_back(std::move(err));
        }
        dispatched.main.errors.clear();
        res.returnType = move(dispatched.returnType);
    }
} Enumerable_toH;

// statically determine things like `Integer === 3` to be true
class Module_tripleEq : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }
        auto rhs = args.args[0]->type;
        if (rhs.isUntyped()) {
            res.returnType = Types::Boolean();
            return;
        }

        auto rhsSym = Symbols::noClassOrModule();
        if (isa_type<ClassType>(rhs)) {
            rhsSym = cast_type_nonnull<ClassType>(rhs).symbol;
        } else if (auto *app = cast_type<AppliedType>(rhs)) {
            rhsSym = app->klass;
        }

        // If at any point we ever change isSubType to transparently convert between sealed classes and a
        // union of subclasses, this code can go away. It's written like this now to limit the blast
        // radius of allocating large union types.
        if (rhsSym.exists() && rhsSym.data(gs)->flags.isSealed && rhsSym.data(gs)->hasSingleSealedSubclass(gs)) {
            rhs = rhsSym.data(gs)->sealedSubclassesToUnion(gs);
        }

        auto rc = Types::getRepresentedClass(gs, args.thisType);
        // in most cases, thisType is T.class_of(rc). see test/testdata/class_not_class_of.rb for an edge case.
        if (rc == core::Symbols::noClassOrModule()) {
            res.returnType = Types::Boolean();
            return;
        }
        auto lhs = rc.data(gs)->externalType();
        ENFORCE(!lhs.isUntyped(), "lhs of Module.=== must be typed");
        if (Types::isSubType(gs, rhs, lhs)) {
            res.returnType = Types::trueClass();
            return;
        }
        if (Types::glb(gs, rhs, lhs).isBottom()) {
            res.returnType = Types::falseClass();
            return;
        }

        res.returnType = Types::Boolean();
    }
} Module_tripleEq;

// Returns true if the type is exactly the class of a specific enum value. For example, given:
//
//   class C < T::Enum
//     enums do
//       X = new
//       Y = new
//     end
//   end
//
//  class TotallyUnrelatedThing ; end
//
// returns true for "X" and "Y", but false for "C", "T::Enum", and "TotallyUnrelatedThing".
static bool isEnumValueClass(const GlobalState &gs, const TypePtr &type) {
    bool must_exist = false;
    auto unwrapped = unwrapSymbol(gs, type, must_exist);

    if (!unwrapped.exists()) {
        return false;
    }

    return unwrapped.data(gs)->name.isTEnumName(gs);
}

class T_Enum_tripleEq : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }
        auto rhs = args.args[0]->type;
        if (rhs.isUntyped()) {
            res.returnType = Types::Boolean();
            return;
        }

        auto rhsSym = Symbols::noClassOrModule();
        if (isa_type<ClassType>(rhs)) {
            rhsSym = cast_type_nonnull<ClassType>(rhs).symbol;
        } else if (auto *app = cast_type<AppliedType>(rhs)) {
            rhsSym = app->klass;
        }

        // If at any point we ever change isSubType to transparently convert between sealed classes and a
        // union of subclasses, this code can go away. It's written like this now to limit the blast
        // radius of allocating large union types.
        if (rhsSym.exists() && rhsSym.data(gs)->flags.isSealed && rhsSym.data(gs)->hasSingleSealedSubclass(gs)) {
            rhs = rhsSym.data(gs)->sealedSubclassesToUnion(gs);
        }

        auto lhs = args.thisType;
        ENFORCE(!lhs.isUntyped(), "lhs of T::Enum.=== must be typed");

        // We have to allow String on the rhs, in order to support legacy_t_enum_migration_mode.
        if (Types::glb(gs, rhs, lhs).isBottom() && Types::glb(gs, rhs, Types::String()).isBottom()) {
            res.returnType = Types::falseClass();
            return;
        }

        if (isEnumValueClass(gs, lhs) && Types::isSubType(gs, rhs, lhs)) {
            res.returnType = Types::trueClass();
            return;
        }

        res.returnType = Types::Boolean();
    }
} T_Enum_tripleEq;

class GenericForwarder_tripleEq : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const override {
        auto forwarderSingleton = Symbols::noClassOrModule();
        if (auto *app = cast_type<AppliedType>(args.thisType)) {
            forwarderSingleton = app->klass;
        } else {
            forwarderSingleton = cast_type_nonnull<ClassType>(args.thisType).symbol;
        }
        auto forwarderSym = forwarderSingleton.data(gs)->attachedClass(gs);

        if (auto e = gs.beginError(args.errLoc(), core::errors::Infer::MetaTypeDispatchCall)) {
            auto realSym = forwarderSym.maybeUnwrapBuiltinGenericForwarder();
            ENFORCE(realSym.exists());
            auto realStr = realSym.show(gs);
            e.setHeader("Use `{}` without any `{}` prefix to match on a stdlib generic type", realStr, "T::");
            auto receiverLoc = args.receiverLoc();
            if (receiverLoc.exists()) {
                e.replaceWith(fmt::format("Replace with {}", realStr), receiverLoc, "{}", realStr);
            }
        }
    }
} GenericForwarder_tripleEq;

const vector<Intrinsic> intrinsics{
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::untyped(), &T_untyped},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::must(), &T_must},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::mustBecause(), &T_must},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::all(), &T_all},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::any(), &T_any},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::nilable(), &T_nilable},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::revealType(), &T_revealType},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::noreturn(), &T_noreturn},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::anything(), &T_anything},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::classOf(), &T_class_of},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::selfType(), &T_self_type},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::attachedClass(), &T_attached_class},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::typeParameter(), &T_type_parameter},

    {Symbols::T(), Intrinsic::Kind::Singleton, Names::proc(), &T_proc},
    {Symbols::DeclBuilderForProcs(), Intrinsic::Kind::Singleton, Names::params(), &T_proc_params},
    {Symbols::DeclBuilderForProcs(), Intrinsic::Kind::Singleton, Names::void_(), &T_proc_void},
    {Symbols::DeclBuilderForProcs(), Intrinsic::Kind::Singleton, Names::returns(), &T_proc_returns},
    {Symbols::DeclBuilderForProcs(), Intrinsic::Kind::Singleton, Names::bind(), &DeclBuilderForProcs_bind},

    {Symbols::T_Generic(), Intrinsic::Kind::Instance, Names::squareBrackets(), &T_Generic_squareBrackets},

    {Symbols::T_Array(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Hash(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Enumerable(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Enumerator(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Enumerator_Lazy(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Enumerator_Chain(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Range(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Set(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Class(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},

    {Symbols::Object(), Intrinsic::Kind::Instance, Names::class_(), &Object_class},
    {Symbols::Object(), Intrinsic::Kind::Instance, Names::singletonClass(), &Object_class},

    {Symbols::Class(), Intrinsic::Kind::Instance, Names::new_(), &Class_new},
    {Symbols::Class(), Intrinsic::Kind::Instance, Names::subclasses(), &Class_subclasses},

    {Symbols::Sorbet_Private_Static(), Intrinsic::Kind::Singleton, Names::sig(), &SorbetPrivateStatic_sig},
    {Symbols::Sorbet_Private_Static_ResolvedSig(), Intrinsic::Kind::Singleton, Names::sig(),
     &SorbetPrivateStaticResolvedSig_sig},

    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::buildHash(), &Magic_buildHashOrKeywordArgs},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::buildArray(), &Magic_buildArray},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::buildRange(), &Magic_buildRange},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::expandSplat(), &Magic_expandSplat},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::callWithSplat(), &Magic_callWithSplat},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::callWithBlock(), &Magic_callWithBlock},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::callWithSplatAndBlock(), &Magic_callWithSplatAndBlock},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::suggestConstantType(), &Magic_suggestUntypedConstantType},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::suggestFieldType(), &Magic_suggestUntypedFieldType},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::attachedClass(), &Magic_attachedClass},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::checkAndAnd(), &Magic_checkAndAnd},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::splat(), &Magic_splat},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::toHashDup(), &Magic_toHash},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::toHashNoDup(), &Magic_toHash},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::mergeHash(), &Magic_mergeHash},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::mergeHashValues(), &Magic_mergeHashValues},

    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::squareBrackets(), &Tuple_squareBrackets},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::first(), &Tuple_first},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::last(), &Tuple_last},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::min(), &Tuple_minMax},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::max(), &Tuple_minMax},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::sum(), &Tuple_sum},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::sample(), &Tuple_sample},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::toA(), &Tuple_to_a},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::concat(), &Tuple_concat},
    // TODO(jez) Array#at intrinsic for tuples

    {Symbols::Shape(), Intrinsic::Kind::Instance, Names::squareBracketsEq(), &Shape_squareBracketsEq},
    {Symbols::Shape(), Intrinsic::Kind::Instance, Names::merge(), &Shape_merge},
    {Symbols::Shape(), Intrinsic::Kind::Instance, Names::toHash(), &Shape_to_hash},

    {Symbols::Hash(), Intrinsic::Kind::Instance, Names::dig(), &Hash_dig},

    {Symbols::Array(), Intrinsic::Kind::Instance, Names::dig(), &Array_dig},
    {Symbols::Array(), Intrinsic::Kind::Instance, Names::flatten(), &Array_flatten},
    {Symbols::Array(), Intrinsic::Kind::Instance, Names::product(), &Array_product},
    {Symbols::Array(), Intrinsic::Kind::Instance, Names::compact(), &Array_compact},
    {Symbols::Array(), Intrinsic::Kind::Instance, Names::zip(), &Array_zip},
    {Symbols::Array(), Intrinsic::Kind::Instance, Names::transpose(), &Array_transpose},

    {Symbols::Symbol(), Intrinsic::Kind::Instance, Names::eqeq(), &Symbol_eqeq},
    {Symbols::String(), Intrinsic::Kind::Instance, Names::eqeq(), &String_eqeq},

    {Symbols::Kernel(), Intrinsic::Kind::Instance, Names::proc(), &Kernel_proc},
    {Symbols::Kernel(), Intrinsic::Kind::Instance, Names::lambda(), &Kernel_proc},
    {Symbols::Kernel(), Intrinsic::Kind::Instance, Names::raise(), &Kernel_raise},
    {Symbols::Kernel(), Intrinsic::Kind::Instance, Names::fail(), &Kernel_raise},

    {Symbols::Enumerable(), Intrinsic::Kind::Instance, Names::toH(), &Enumerable_toH},

    {Symbols::Module(), Intrinsic::Kind::Instance, Names::tripleEq(), &Module_tripleEq},
    {Symbols::T_Enum(), Intrinsic::Kind::Instance, Names::tripleEq(), &T_Enum_tripleEq},

    {Symbols::T_Array(), Intrinsic::Kind::Singleton, Names::tripleEq(), &GenericForwarder_tripleEq},
    {Symbols::T_Hash(), Intrinsic::Kind::Singleton, Names::tripleEq(), &GenericForwarder_tripleEq},
    {Symbols::T_Enumerable(), Intrinsic::Kind::Singleton, Names::tripleEq(), &GenericForwarder_tripleEq},
    {Symbols::T_Enumerator(), Intrinsic::Kind::Singleton, Names::tripleEq(), &GenericForwarder_tripleEq},
    {Symbols::T_Enumerator_Lazy(), Intrinsic::Kind::Singleton, Names::tripleEq(), &GenericForwarder_tripleEq},
    {Symbols::T_Enumerator_Chain(), Intrinsic::Kind::Singleton, Names::tripleEq(), &GenericForwarder_tripleEq},
    {Symbols::T_Range(), Intrinsic::Kind::Singleton, Names::tripleEq(), &GenericForwarder_tripleEq},
    {Symbols::T_Set(), Intrinsic::Kind::Singleton, Names::tripleEq(), &GenericForwarder_tripleEq},
    {Symbols::T_Class(), Intrinsic::Kind::Singleton, Names::tripleEq(), &GenericForwarder_tripleEq},
};

UnorderedMap<NameRef, const vector<NameRef>> computeIntrinsicsDispatchMap() {
    UnorderedMap<NameRef, const vector<NameRef>> result;
    for (const auto &intrinsic : intrinsics) {
        auto targets = intrinsic.impl->dispatchesTo();
        if (targets.empty()) {
            continue;
        }

        result.emplace(intrinsic.method, move(targets));
    }
    return result;
}

const IntrinsicMethodsDispatchMap intrinsicsDispatchMap = computeIntrinsicsDispatchMap();

} // namespace

absl::Span<const Intrinsic> intrinsicMethods() {
    return absl::MakeSpan(intrinsics);
}

const IntrinsicMethodsDispatchMap &intrinsicMethodsDispatchMap() {
    return intrinsicsDispatchMap;
}

} // namespace sorbet::core
