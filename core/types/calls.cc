#include "absl/strings/match.h"
#include "absl/strings/str_split.h"
#include "common/common.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "core/GlobalState.h"
#include "core/Names.h"
#include "core/Symbols.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"
#include "core/errors/infer.h"
#include "core/errors/resolver.h"
#include <algorithm> // find_if, sort

#include "absl/strings/str_cat.h"

template class std::vector<sorbet::core::SymbolRef>;
using namespace std;

namespace sorbet::core {

DispatchResult ProxyType::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "proxytype");
    auto und = underlying();
    return und->dispatchCall(gs, args);
}

TypePtr ProxyType::getCallArguments(const GlobalState &gs, NameRef name) {
    return underlying()->getCallArguments(gs, name);
}

DispatchResult OrType::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "ortype");
    auto leftRet = left->dispatchCall(gs, args.withSelfRef(left));
    auto rightRet = right->dispatchCall(gs, args.withSelfRef(right));
    DispatchResult ret{Types::any(gs, leftRet.returnType, rightRet.returnType), move(leftRet.main),
                       make_unique<DispatchResult>(move(rightRet)), DispatchResult::Combinator::OR};
    return ret;
}

TypePtr OrType::getCallArguments(const GlobalState &gs, NameRef name) {
    auto largs = left->getCallArguments(gs, name);
    auto rargs = right->getCallArguments(gs, name);
    if (!largs) {
        largs = Types::untypedUntracked();
    }
    if (!rargs) {
        rargs = Types::untypedUntracked();
    }
    return Types::glb(gs, largs, rargs);
}

DispatchResult TypeVar::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    Exception::raise("should never happen");
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

DispatchResult AndType::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "andtype");
    auto leftRet = left->dispatchCall(gs, args);
    auto rightRet = right->dispatchCall(gs, args);

    // If either side is missing the method, dispatch to the other.
    auto leftOk = allComponentsPresent(leftRet);
    auto rightOk = allComponentsPresent(rightRet);
    if (leftOk && !rightOk) {
        return leftRet;
    }
    if (rightOk && !leftOk) {
        return rightRet;
    }
    DispatchResult ret{Types::all(gs, leftRet.returnType, rightRet.returnType), move(leftRet.main),
                       make_unique<DispatchResult>(move(rightRet)),

                       DispatchResult::Combinator::AND};

    return ret;
}

TypePtr AndType::getCallArguments(const GlobalState &gs, NameRef name) {
    auto l = left->getCallArguments(gs, name);
    auto r = right->getCallArguments(gs, name);
    if (l == nullptr) {
        return r;
    }
    if (r == nullptr) {
        return l;
    }
    return Types::any(gs, l, r);
}

DispatchResult ShapeType::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "shapetype");
    auto method = Symbols::Shape().data(gs)->findMember(gs, args.name);
    if (method.exists() && method.data(gs)->intrinsic != nullptr) {
        DispatchComponent comp{args.selfType, method, {}, nullptr, nullptr, nullptr, ArgInfo{}, nullptr};
        DispatchResult res{nullptr, std::move(comp)};
        method.data(gs)->intrinsic->apply(gs, args, this, res);
        if (res.returnType != nullptr) {
            return res;
        }
    }
    return ProxyType::dispatchCall(gs, args);
}

DispatchResult TupleType::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "tupletype");
    auto method = Symbols::Tuple().data(gs)->findMember(gs, args.name);
    if (method.exists() && method.data(gs)->intrinsic != nullptr) {
        DispatchComponent comp{args.selfType, method, {}, nullptr, nullptr, nullptr, ArgInfo{}, nullptr};
        DispatchResult res{nullptr, std::move(comp)};
        method.data(gs)->intrinsic->apply(gs, args, this, res);
        if (res.returnType != nullptr) {
            return res;
        }
    }
    return ProxyType::dispatchCall(gs, args);
}

namespace {
bool isSetter(const GlobalState &gs, NameRef fun) {
    if (fun.data(gs)->kind != NameKind::UTF8) {
        return false;
    }
    const string_view rawName = fun.data(gs)->raw.utf8;
    if (rawName.size() < 2) {
        return false;
    }
    if (rawName.back() == '=') {
        return !(fun == Names::leq() || fun == Names::geq() || fun == Names::tripleEq() || fun == Names::eqeq() ||
                 fun == Names::neq());
    }
    return false;
}

u4 locSize(core::Loc loc) {
    return loc.endPos() - loc.beginPos();
}

// Find the smallest applicable arg loc that falls within the callLoc. Returns the call site's loc if none found.
// Used to ignore origins that are not relevant to call site.
core::Loc smallestLocWithin(core::Loc callLoc, const core::TypeAndOrigins &argTpe) {
    core::Loc chosen = callLoc;
    for (auto loc : argTpe.origins) {
        if (callLoc.contains(loc) && locSize(loc) < locSize(chosen)) {
            chosen = loc;
        }
    }
    return chosen;
}

unique_ptr<Error> matchArgType(const GlobalState &gs, TypeConstraint &constr, Loc callLoc, Loc receiverLoc,
                               SymbolRef inClass, SymbolRef method, const TypeAndOrigins &argTpe, const ArgInfo &argSym,
                               const TypePtr &selfType, vector<TypePtr> &targs, Loc loc, bool mayBeSetter = false) {
    TypePtr expectedType = Types::resultTypeAsSeenFrom(gs, argSym.type, method.data(gs)->owner, inClass, targs);
    if (!expectedType) {
        expectedType = Types::untyped(gs, method);
    }

    expectedType = Types::replaceSelfType(gs, expectedType, selfType);

    if (Types::isSubTypeUnderConstraint(gs, constr, argTpe.type, expectedType, UntypedMode::AlwaysCompatible)) {
        return nullptr;
    }

    if (auto e = gs.beginError(smallestLocWithin(callLoc, argTpe), errors::Infer::MethodArgumentMismatch)) {
        if (mayBeSetter && isSetter(gs, method.data(gs)->name)) {
            e.setHeader("Assigning a value to `{}` that does not match expected type `{}`", argSym.argumentName(gs),
                        expectedType->show(gs));
        } else {
            e.setHeader("Expected `{}` but found `{}` for argument `{}`", expectedType->show(gs), argTpe.type->show(gs),
                        argSym.argumentName(gs));
            e.addErrorSection(ErrorSection({
                ErrorLine::from(argSym.loc, "Method `{}` has specified `{}` as `{}`", method.data(gs)->show(gs),
                                argSym.argumentName(gs), expectedType->show(gs)),
            }));
        }
        e.addErrorSection(
            ErrorSection("Got " + argTpe.type->show(gs) + " originating from:", argTpe.origins2Explanations(gs)));
        auto withoutNil = Types::approximateSubtract(gs, argTpe.type, Types::nilClass());
        if (!withoutNil->isBottom() &&
            Types::isSubTypeUnderConstraint(gs, constr, withoutNil, expectedType, UntypedMode::AlwaysCompatible)) {
            if (loc.exists()) {
                e.replaceWith("Wrap in `T.must`", loc, "T.must({})", loc.source(gs));
            }
        }
        return e.build();
    }
    return nullptr;
}

unique_ptr<Error> missingArg(const GlobalState &gs, Loc callLoc, Loc receiverLoc, SymbolRef method,
                             const ArgInfo &arg) {
    if (auto e = gs.beginError(callLoc, errors::Infer::MethodArgumentCountMismatch)) {
        e.setHeader("Missing required keyword argument `{}` for method `{}`", arg.name.show(gs),
                    method.data(gs)->show(gs));
        return e.build();
    }
    return nullptr;
}
}; // namespace

int getArity(const GlobalState &gs, SymbolRef method) {
    ENFORCE(!method.data(gs)->arguments().empty(), "Every method should have at least a block arg.");
    ENFORCE(method.data(gs)->arguments().back().flags.isBlock, "Last arg should be the block arg.");

    // Don't count the block arg in the arity
    return method.data(gs)->arguments().size() - 1;
}

// Guess overload. The way we guess is only arity based - we will return the overload that has the smallest number of
// arguments that is >= args.size()
SymbolRef guessOverload(const GlobalState &gs, SymbolRef inClass, SymbolRef primary,
                        InlinedVector<const TypeAndOrigins *, 2> &args, const TypePtr &fullType, vector<TypePtr> &targs,
                        bool hasBlock) {
    counterInc("calls.overloaded_invocations");
    ENFORCE(Context::permitOverloadDefinitions(gs, primary.data(gs)->loc().file(), primary),
            "overload not permitted here");
    SymbolRef fallback = primary;
    vector<SymbolRef> allCandidates;

    allCandidates.emplace_back(primary);
    { // create candidates and sort them by number of arguments(stable by symbol id)
        int i = 0;
        SymbolRef current = primary;
        while (current.data(gs)->isOverloaded()) {
            i++;
            NameRef overloadName = gs.lookupNameUnique(UniqueNameKind::Overload, primary.data(gs)->name, i);
            SymbolRef overload = primary.data(gs)->owner.data(gs)->findMember(gs, overloadName);
            if (!overload.exists()) {
                Exception::raise("Corruption of overloads?");
            } else {
                allCandidates.emplace_back(overload);
                current = overload;
            }
        }

        fast_sort(allCandidates, [&](SymbolRef s1, SymbolRef s2) -> bool {
            if (getArity(gs, s1) < getArity(gs, s2)) {
                return true;
            }
            if (getArity(gs, s1) == getArity(gs, s2)) {
                return s1.rawId() < s2.rawId();
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
                if (i >= getArity(gs, candidate)) {
                    it = leftCandidates.erase(it);
                    continue;
                }

                auto argType = Types::resultTypeAsSeenFrom(gs, candidate.data(gs)->arguments()[i].type,
                                                           candidate.data(gs)->owner, inClass, targs);
                if (argType->isFullyDefined() && !Types::isSubType(gs, arg->type, argType)) {
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
            const auto &args = candidate.data(gs)->arguments();
            ENFORCE(!args.empty(), "Should at least have a block argument.");
            auto mentionsBlockArg = !args.back().isSyntheticBlockArgument();
            if (mentionsBlockArg != hasBlock) {
                it = leftCandidates.erase(it);
                continue;
            }
            ++it;
        }
    }

    { // keep only candidates with closest arity
        struct Comp {
            const GlobalState &gs;

            bool operator()(SymbolRef s, int i) const {
                return getArity(gs, s) < i;
            }

            bool operator()(int i, SymbolRef s) const {
                return i < getArity(gs, s);
            }

            Comp(const GlobalState &gs) : gs(gs){};
        } cmp(gs);

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

/**
 * unwrapType is used to take an expression that's parsed at the value-level,
 * and turn it into a type. For example, consider the following two expressions:
 *
 * > Integer.sqrt 10
 * > T::Array[Integer].new
 *
 * In both lines, `Integer` is initially resolved as the singleton class of
 * `Integer`. This is because it's not immediately clear if we want to refer
 * to the type `Integer` or if we want the singleton class of Integer for
 * calling singleton methods. In the first line this was the correct choice, as
 * we're just invoking the singleton method `sqrt`. In the second case we need
 * to fix up the `Integer` sub-expression, and turn it back into the type of
 * integer values. This is what `unwrapType` does, it turns the value-level
 * expression back into a type-level one.
 */
TypePtr unwrapType(const GlobalState &gs, Loc loc, const TypePtr &tp) {
    if (auto *metaType = cast_type<MetaType>(tp.get())) {
        return metaType->wrapped;
    }

    if (auto *classType = cast_type<ClassType>(tp.get())) {
        if (classType->symbol.data(gs)->derivesFrom(gs, core::Symbols::T_Enum())) {
            // T::Enum instances are allowed to stand for themselves in type syntax positions.
            // See the note in type_syntax.cc regarding T::Enum.
            return tp;
        }

        SymbolRef attachedClass = classType->symbol.data(gs)->attachedClass(gs);
        if (!attachedClass.exists()) {
            if (auto e = gs.beginError(loc, errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
            }
            return Types::untypedUntracked();
        }

        return attachedClass.data(gs)->externalType(gs);
    }

    if (auto *appType = cast_type<AppliedType>(tp.get())) {
        SymbolRef attachedClass = appType->klass.data(gs)->attachedClass(gs);
        if (!attachedClass.exists()) {
            if (auto e = gs.beginError(loc, errors::Infer::BareTypeUsage)) {
                e.setHeader("Unsupported usage of bare type");
            }
            return Types::untypedUntracked();
        }

        return attachedClass.data(gs)->externalType(gs);
    }

    if (auto *shapeType = cast_type<ShapeType>(tp.get())) {
        vector<TypePtr> unwrappedValues;
        unwrappedValues.reserve(shapeType->values.size());
        for (auto value : shapeType->values) {
            unwrappedValues.emplace_back(unwrapType(gs, loc, value));
        }
        return make_type<ShapeType>(Types::hashOfUntyped(), shapeType->keys, unwrappedValues);
    } else if (auto *tupleType = cast_type<TupleType>(tp.get())) {
        vector<TypePtr> unwrappedElems;
        unwrappedElems.reserve(tupleType->elems.size());
        for (auto elem : tupleType->elems) {
            unwrappedElems.emplace_back(unwrapType(gs, loc, elem));
        }
        return TupleType::build(gs, unwrappedElems);
    } else if (auto *litType = cast_type<LiteralType>(tp.get())) {
        if (auto e = gs.beginError(loc, errors::Infer::BareTypeUsage)) {
            e.setHeader("Unsupported usage of literal type");
        }
        return Types::untypedUntracked();
    }
    return tp;
}

string prettyArity(const GlobalState &gs, SymbolRef method) {
    int required = 0, optional = 0;
    bool repeated = false;
    for (const auto &arg : method.data(gs)->arguments()) {
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
    if (repeated) {
        return absl::StrCat(required, "+");
    } else if (optional > 0) {
        return absl::StrCat(required, "..", required + optional);
    } else {
        return to_string(required);
    }
}

bool extendsTHelpers(const GlobalState &gs, core::SymbolRef enclosingClass) {
    ENFORCE(enclosingClass.exists());
    auto enclosingSingletonClass = enclosingClass.data(gs)->lookupSingletonClass(gs);
    ENFORCE(enclosingSingletonClass.exists());
    return enclosingSingletonClass.data(gs)->derivesFrom(gs, core::Symbols::T_Helpers());
}

/**
 * Make an autocorrection for adding `extend T::Helpers`, when needed.
 */
optional<core::AutocorrectSuggestion> maybeSuggestExtendTHelpers(const GlobalState &gs, core::SymbolRef enclosingClass,
                                                                 const Loc &call) {
    if (extendsTHelpers(gs, enclosingClass)) {
        // No need to suggest here, because it already has 'extend T::Sig'
        return nullopt;
    }

    auto inFileOfMethod = [&](const auto &loc) { return loc.file() == call.file(); };
    auto classLocs = enclosingClass.data(gs)->locs();
    auto classLoc = absl::c_find_if(classLocs, inFileOfMethod);

    if (classLoc == classLocs.end()) {
        // Couldn't a loc for the enclosing class in this file, give up.
        return nullopt;
    }

    auto [classStart, classEnd] = classLoc->position(gs);

    core::Loc::Detail thisLineStart = {classStart.line, 1};
    auto thisLineLoc = core::Loc::fromDetails(gs, classLoc->file(), thisLineStart, thisLineStart);
    ENFORCE(thisLineLoc.has_value());
    auto [_, thisLinePadding] = thisLineLoc.value().findStartOfLine(gs);

    core::Loc::Detail nextLineStart = {classStart.line + 1, 1};
    auto nextLineLoc = core::Loc::fromDetails(gs, classLoc->file(), nextLineStart, nextLineStart);
    if (!nextLineLoc.has_value()) {
        return nullopt;
    }
    auto [replacementLoc, nextLinePadding] = nextLineLoc.value().findStartOfLine(gs);

    // Preserve the indentation of the line below us.
    string prefix(max(thisLinePadding + 2, nextLinePadding), ' ');
    return core::AutocorrectSuggestion{
        "Add `extend T::Helpers`",
        {core::AutocorrectSuggestion::Edit{nextLineLoc.value(), fmt::format("{}extend T::Helpers\n", prefix)}}};
}

// This implements Ruby's argument matching logic (assigning values passed to a
// method call to formal parameters of the method).
//
// Known incompleteness or inconsistencies with Ruby:
//  - Missing coercion to keyword arguments via `#to_hash`
//  - We never allow a non-shaped Hash to satisfy keyword arguments;
//    We should, at a minimum, probably allow one to satisfy an **kwargs : untyped
//    (with a subtype check on the key type, once we have generics)
DispatchResult dispatchCallSymbol(const GlobalState &gs, DispatchArgs args,

                                  const Type *thisType, core::SymbolRef symbol, vector<TypePtr> &targs) {
    if (symbol == core::Symbols::untyped()) {
        return DispatchResult(Types::untyped(gs, thisType->untypedBlame()), std::move(args.selfType),
                              Symbols::untyped());
    } else if (symbol == Symbols::void_()) {
        if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.call), errors::Infer::UnknownMethod)) {
            e.setHeader("Can not call method `{}` on void type", args.name.data(gs)->show(gs));
        }
        return DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noSymbol());
    }

    SymbolRef mayBeOverloaded = symbol.data(gs)->findMemberTransitive(gs, args.name);

    if (!mayBeOverloaded.exists()) {
        if (args.name == Names::initialize()) {
            // Special-case initialize(). We should define this on
            // `BasicObject`, but our method-resolution order is wrong, and
            // putting it there will inadvertently shadow real definitions in
            // some cases, so we special-case it here as a last resort.
            auto result = DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noSymbol());
            if (!args.args.empty()) {
                if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.call),
                                           errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Wrong number of arguments for constructor. Expected: `{}`, got: `{}`", 0,
                                args.args.size());
                    result.main.errors.emplace_back(e.build());
                }
            }
            return result;
        } else if (args.name == core::Names::super()) {
            return DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::untyped());
        }
        auto result = DispatchResult(Types::untypedUntracked(), std::move(args.selfType), Symbols::noSymbol());
        // This is a hack. We want to always be able to build the error object
        // so that it is not immediately sent to GlobalState::_error
        // and recorded.
        // Instead, the error always should get queued up in the
        // errors list of the result so that the caller can deal with the error.
        auto e = gs.beginError(core::Loc(args.locs.file, args.locs.call), errors::Infer::UnknownMethod);
        if (e) {
            string thisStr = thisType->show(gs);
            if (args.fullType.get() != thisType) {
                e.setHeader("Method `{}` does not exist on `{}` component of `{}`", args.name.data(gs)->show(gs),
                            thisType->show(gs), args.fullType->show(gs));
            } else {
                e.setHeader("Method `{}` does not exist on `{}`", args.name.data(gs)->show(gs), thisStr);

                // catch the special case of `interface!`, `abstract!`, `final!`, or `sealed!` and
                // suggest adding `extend T::Helpers`.
                if (args.name == core::Names::declareInterface() || args.name == core::Names::declareAbstract() ||
                    args.name == core::Names::declareFinal() || args.name == core::Names::declareSealed() ||
                    args.name == core::Names::mixesInClassMethods()) {
                    auto attachedClass = symbol.data(gs)->attachedClass(gs);
                    if (auto suggestion =
                            maybeSuggestExtendTHelpers(gs, attachedClass, core::Loc(args.locs.file, args.locs.call))) {
                        e.addAutocorrect(std::move(*suggestion));
                    }
                }
            }
            if (args.fullType.get() != thisType && symbol == Symbols::NilClass()) {
                e.replaceWith("Wrap in `T.must`", core::Loc(args.locs.file, args.locs.receiver), "T.must({})",
                              core::Loc(args.locs.file, args.locs.receiver).source(gs));
            } else {
                if (symbol.data(gs)->isClassOrModuleModule()) {
                    auto objMeth = core::Symbols::Object().data(gs)->findMemberTransitive(gs, args.name);
                    if (objMeth.exists() && objMeth.data(gs)->owner.data(gs)->isClassOrModuleModule()) {
                        e.addErrorSection(
                            ErrorSection(ErrorColors::format("Did you mean to `include {}` in this module?",
                                                             objMeth.data(gs)->owner.data(gs)->name.show(gs))));
                    }
                }
                auto alternatives = symbol.data(gs)->findMemberFuzzyMatch(gs, args.name);
                if (!alternatives.empty()) {
                    vector<ErrorLine> lines;
                    lines.reserve(alternatives.size());
                    for (auto alternative : alternatives) {
                        auto possibleSymbol = alternative.symbol.data(gs);
                        if (!possibleSymbol->isClassOrModule() && !possibleSymbol->isMethod()) {
                            continue;
                        }

                        auto suggestedName = possibleSymbol->isClassOrModule() ? alternative.symbol.show(gs) + ".new"
                                                                               : alternative.symbol.show(gs);

                        bool addedAutocorrect = false;
                        if (possibleSymbol->isClassOrModule()) {
                            const auto replacement = possibleSymbol->name.show(gs);
                            const auto loc = core::Loc(args.locs.file, args.locs.call);
                            const auto toReplace = args.name.toString(gs);
                            // This is a bit hacky but the loc corresponding to the send isn't available here and until
                            // it is, this verifies that the methodLoc below exists.
                            if (absl::StartsWith(loc.source(gs), toReplace)) {
                                const auto methodLoc =
                                    Loc{loc.file(), loc.beginPos(), (u4)(loc.beginPos() + toReplace.length())};
                                e.replaceWith(fmt::format("Replace with `{}.new`", replacement), methodLoc, "{}.new",
                                              replacement);
                                addedAutocorrect = true;
                            }
                        } else {
                            const auto replacement = possibleSymbol->name.toString(gs);
                            const auto toReplace = args.name.toString(gs);
                            if (replacement != toReplace) {
                                const auto loc = core::Loc(args.locs.file, args.locs.receiver);
                                // See comment above.
                                if (absl::StartsWith(core::Loc(args.locs.file, args.locs.call).source(gs),
                                                     fmt::format("{}.{}", loc.source(gs), toReplace))) {
                                    const auto methodLoc =
                                        Loc{loc.file(), loc.endPos() + 1, (u4)(loc.endPos() + 1 + toReplace.length())};
                                    e.replaceWith(fmt::format("Replace with `{}`", replacement), methodLoc, "{}",
                                                  replacement);
                                    addedAutocorrect = true;
                                }
                            }
                        }

                        if (!addedAutocorrect) {
                            lines.emplace_back(ErrorLine::from(alternative.symbol.data(gs)->loc(),
                                                               "Did you mean: `{}`?", suggestedName));
                        }
                    }
                    e.addErrorSection(ErrorSection(lines));
                }

                auto attached = symbol.data(gs)->attachedClass(gs);
                if (attached.exists() && symbol.data(gs)->derivesFrom(gs, Symbols::Chalk_Tools_Accessible())) {
                    e.addErrorSection(ErrorSection(
                        "If this method is generated by Chalk::Tools::Accessible, you "
                        "may need to re-generate the .rbi. Try running:\n" +
                        ErrorColors::format("  scripts/bin/remote-script sorbet/shim_generation/make_accessible.rb {}",
                                            attached.data(gs)->showFullName(gs))));
                }
            }
        }
        result.main.errors.emplace_back(e.build());
        return result;
    }

    SymbolRef method =
        mayBeOverloaded.data(gs)->isOverloaded()
            ? guessOverload(gs, symbol, mayBeOverloaded, args.args, args.fullType, targs, args.block != nullptr)
            : mayBeOverloaded;

    DispatchResult result;
    auto &component = result.main;
    component.receiver = args.selfType;
    component.method = method;

    const SymbolData data = method.data(gs);
    unique_ptr<TypeConstraint> &maybeConstraint = result.main.constr;
    TypeConstraint *constr;
    if (args.block || data->isGenericMethod()) {
        maybeConstraint = make_unique<TypeConstraint>();
        constr = maybeConstraint.get();
    } else {
        constr = &TypeConstraint::EmptyFrozenConstraint;
    }

    if (data->isGenericMethod()) {
        constr->defineDomain(gs, data->typeArguments());
    }
    bool hasKwargs = absl::c_any_of(data->arguments(), [](const auto &arg) { return arg.flags.isKeyword; });

    // p -> params, i.e., what was mentioned in the defintiion
    auto pit = data->arguments().begin();
    auto pend = data->arguments().end();

    ENFORCE(pit != pend, "Should at least have the block arg.");
    ENFORCE((pend - 1)->flags.isBlock, "Last arg should be the block arg: " + (pend - 1)->show(gs));
    // We'll type check the block arg separately from the rest of the args.
    --pend;

    // a -> args, i.e., what was passed at the call site
    auto ait = args.args.begin();
    auto aend = args.args.end();

    while (pit != pend && ait != aend) {
        const ArgInfo &spec = *pit;
        auto &arg = *ait;
        if (spec.flags.isKeyword) {
            break;
        }
        if (ait + 1 == aend && hasKwargs && (spec.flags.isDefault || spec.flags.isRepeated) &&
            Types::approximate(gs, arg->type, *constr)->derivesFrom(gs, Symbols::Hash())) {
            break;
        }

        auto offset = ait - args.args.begin();
        if (auto e =
                matchArgType(gs, *constr, core::Loc(args.locs.file, args.locs.call),
                             core::Loc(args.locs.file, args.locs.receiver), symbol, method, *arg, spec, args.selfType,
                             targs, core::Loc(args.locs.file, args.locs.args[offset]), args.args.size() == 1)) {
            result.main.errors.emplace_back(std::move(e));
        }

        if (!spec.flags.isRepeated) {
            ++pit;
        }
        ++ait;
    }

    if (pit != pend) {
        if (!(pit->flags.isKeyword || pit->flags.isDefault || pit->flags.isRepeated || pit->flags.isBlock)) {
            if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.call),
                                       errors::Infer::MethodArgumentCountMismatch)) {
                if (args.fullType.get() != thisType) {
                    e.setHeader(
                        "Not enough arguments provided for method `{}` on `{}` component of `{}`. Expected: `{}`, got: "
                        "`{}`",
                        data->show(gs), thisType->show(gs), args.fullType->show(gs), prettyArity(gs, method),
                        args.args.size()); // TODO: should use position and print the source tree, not the cfg one.
                } else {
                    e.setHeader(
                        "Not enough arguments provided for method `{}`. Expected: `{}`, got: `{}`", data->show(gs),
                        prettyArity(gs, method),
                        args.args.size()); // TODO: should use position and print the source tree, not the cfg one.
                }
                e.addErrorLine(method.data(gs)->loc(), "`{}` defined here", data->show(gs));
                if (args.name == core::Names::any() &&
                    symbol == core::Symbols::T().data(gs)->lookupSingletonClass(gs)) {
                    e.addErrorSection(
                        core::ErrorSection("Hint: if you want to allow any type as an argument, use `T.untyped`"));
                }

                result.main.errors.emplace_back(e.build());
            }
        }
    }

    // keep this around so we know which keyword arguments have been supplied
    UnorderedSet<NameRef> consumed;
    if (hasKwargs && ait != aend) {
        auto &hashArg = *(aend - 1);
        auto hashArgType = Types::approximate(gs, hashArg->type, *constr);

        // find keyword arguments and advance `pend` before them; We'll walk
        // `kwit` ahead below
        auto kwit = pit;
        while (!kwit->flags.isKeyword) {
            kwit++;
        }
        pend = kwit;
        if (hashArgType->isUntyped()) {
            // Allow an untyped arg to satisfy all kwargs
            --aend;
        } else if (auto *hash = cast_type<ShapeType>(hashArgType.get())) {
            --aend;

            while (kwit != data->arguments().end()) {
                const ArgInfo &spec = *kwit;
                if (spec.flags.isBlock) {
                    break;
                } else if (spec.flags.isRepeated) {
                    for (auto it = hash->keys.begin(); it != hash->keys.end(); ++it) {
                        auto key = cast_type<LiteralType>(it->get());
                        SymbolRef klass = cast_type<ClassType>(key->underlying().get())->symbol;
                        if (klass != Symbols::Symbol()) {
                            continue;
                        }

                        NameRef arg(gs, key->value);
                        if (consumed.find(NameRef(gs, key->value)) != consumed.end()) {
                            continue;
                        }
                        consumed.insert(arg);

                        TypeAndOrigins tpe;
                        tpe.origins = args.args.back()->origins;
                        auto offset = it - hash->keys.begin();
                        tpe.type = hash->values[offset];
                        if (auto e = matchArgType(gs, *constr, core::Loc(args.locs.file, args.locs.call),
                                                  core::Loc(args.locs.file, args.locs.receiver), symbol, method, tpe,
                                                  spec, args.selfType, targs, Loc::none())) {
                            result.main.errors.emplace_back(std::move(e));
                        }
                    }
                    break;
                }
                ++kwit;

                auto arg = absl::c_find_if(hash->keys, [&](const TypePtr &litType) {
                    auto lit = cast_type<LiteralType>(litType.get());
                    return cast_type<ClassType>(lit->underlying().get())->symbol == Symbols::Symbol() &&
                           lit->value == spec.name._id;
                });
                if (arg == hash->keys.end()) {
                    if (!spec.flags.isDefault) {
                        if (auto e = missingArg(gs, core::Loc(args.locs.file, args.locs.call),
                                                core::Loc(args.locs.file, args.locs.receiver), method, spec)) {
                            result.main.errors.emplace_back(std::move(e));
                        }
                    }
                    continue;
                }
                consumed.insert(spec.name);
                TypeAndOrigins tpe;
                tpe.origins = args.args.back()->origins;
                auto offset = arg - hash->keys.begin();
                tpe.type = hash->values[offset];
                if (auto e = matchArgType(gs, *constr, core::Loc(args.locs.file, args.locs.call),
                                          core::Loc(args.locs.file, args.locs.receiver), symbol, method, tpe, spec,
                                          args.selfType, targs, Loc::none())) {
                    result.main.errors.emplace_back(std::move(e));
                }
            }
            for (auto &keyType : hash->keys) {
                auto key = cast_type<LiteralType>(keyType.get());
                SymbolRef klass = cast_type<ClassType>(key->underlying().get())->symbol;
                if (klass == Symbols::Symbol() && consumed.find(NameRef(gs, key->value)) != consumed.end()) {
                    continue;
                }
                NameRef arg(gs, key->value);

                if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.call),
                                           errors::Infer::MethodArgumentCountMismatch)) {
                    e.setHeader("Unrecognized keyword argument `{}` passed for method `{}`", arg.show(gs),
                                data->show(gs));
                    result.main.errors.emplace_back(e.build());
                }
            }
        } else if (hashArgType->derivesFrom(gs, Symbols::Hash())) {
            --aend;
            if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.call), errors::Infer::UntypedSplat)) {
                e.setHeader("Passing a hash where the specific keys are unknown to a method taking keyword arguments");
                e.addErrorSection(ErrorSection("Got " + hashArgType->show(gs) + " originating from:",
                                               hashArg->origins2Explanations(gs)));
                result.main.errors.emplace_back(e.build());
            }
        }
    }
    if (hasKwargs && aend == args.args.end()) {
        // We have keyword arguments, but we didn't consume a hash at the
        // end. Report an error for each missing required keyword arugment.
        for (auto &spec : data->arguments()) {
            if (!spec.flags.isKeyword || spec.flags.isDefault || spec.flags.isRepeated) {
                continue;
            }
            if (auto e = missingArg(gs, core::Loc(args.locs.file, args.locs.call),
                                    core::Loc(args.locs.file, args.locs.receiver), method, spec)) {
                result.main.errors.emplace_back(std::move(e));
            }
        }
    }

    if (ait != aend) {
        if (auto e =
                gs.beginError(core::Loc(args.locs.file, args.locs.call), errors::Infer::MethodArgumentCountMismatch)) {
            if (!hasKwargs) {
                e.setHeader("Too many arguments provided for method `{}`. Expected: `{}`, got: `{}`", data->show(gs),
                            prettyArity(gs, method), args.args.size());
                e.addErrorLine(method.data(gs)->loc(), "`{}` defined here", args.name.show(gs));
            } else {
                // if we have keyword arguments, we should print a more informative message: otherwise, we might give
                // people some slightly confusing error messages.

                // count the number of arguments
                int posArgs = args.args.size();
                // and if we have keyword arguments (i.e. if the last argument is a hash) then subtract 1 to get the
                // total number of positional arguments
                if (posArgs > 0 && isa_type<ShapeType>(args.args.back()->type.get())) {
                    posArgs--;
                }
                // print a helpful error message
                e.setHeader("Too many positional arguments provided for method `{}`. Expected: `{}`, got: `{}`",
                            data->show(gs), prettyArity(gs, method), posArgs);
                e.addErrorLine(method.data(gs)->loc(), "`{}` defined here", args.name.show(gs));

                // if there's an obvious first keyword argument that the user hasn't supplied, we can mention it
                // explicitly
                auto firstKeyword = absl::c_find_if(data->arguments(), [&consumed](const ArgInfo &arg) {
                    return arg.flags.isKeyword && arg.flags.isDefault && consumed.count(arg.name) == 0;
                });
                if (firstKeyword != data->arguments().end()) {
                    e.addErrorLine(core::Loc(args.locs.file, args.locs.call),
                                   "`{}` has optional keyword arguments. Did you mean to provide a value for `{}`?",
                                   data->show(gs), firstKeyword->argumentName(gs));
                }
            }
            result.main.errors.emplace_back(e.build());
        }
    }

    if (args.block != nullptr) {
        ENFORCE(!data->arguments().empty(), "Every symbol must at least have a block arg: {}", data->show(gs));
        const auto &bspec = data->arguments().back();
        ENFORCE(bspec.flags.isBlock, "The last symbol must be the block arg: {}", data->show(gs));

        TypePtr blockType = Types::resultTypeAsSeenFrom(gs, bspec.type, data->owner, symbol, targs);
        if (!blockType) {
            blockType = Types::untyped(gs, method);
        }

        component.blockReturnType = Types::getProcReturnType(gs, Types::dropNil(gs, blockType));
        blockType = constr->isSolved() ? Types::instantiate(gs, blockType, *constr)
                                       : Types::approximate(gs, blockType, *constr);
        component.blockPreType = blockType;
        component.blockSpec = bspec.deepCopy();
    }

    TypePtr &resultType = result.returnType;

    if (method.data(gs)->intrinsic != nullptr) {
        method.data(gs)->intrinsic->apply(gs, args, thisType, result);
        // the call could have overriden constraint
        if (result.main.constr || constr != &core::TypeConstraint::EmptyFrozenConstraint) {
            constr = result.main.constr.get();
        }
        if (constr == nullptr) {
            constr = &core::TypeConstraint::EmptyFrozenConstraint;
        }
    }

    if (resultType == nullptr) {
        if (args.args.size() == 1 && isSetter(gs, method.data(gs)->name)) {
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
        // if block is there we do not attempt to solve the constaint. CFG adds an explicit solve
        // node that triggers constraint solving
        if (!constr->solve(gs)) {
            if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.call),
                                       errors::Infer::GenericMethodConstaintUnsolved)) {
                e.setHeader("Could not find valid instantiation of type parameters");
                result.main.errors.emplace_back(e.build());
            }
        }
        ENFORCE(!data->arguments().empty(), "Every method should at least have a block arg.");
        ENFORCE(data->arguments().back().flags.isBlock, "The last arg should be the block arg.");
        auto blockType = data->arguments().back().type;
        if (blockType && !core::Types::isSubType(gs, core::Types::nilClass(), blockType)) {
            if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.call), errors::Infer::BlockNotPassed)) {
                e.setHeader("`{}` requires a block parameter, but no block was passed", args.name.show(gs));
                e.addErrorLine(method.data(gs)->loc(), "defined here");
                result.main.errors.emplace_back(e.build());
            }
        }
    }

    if (!resultType) {
        resultType = Types::untyped(gs, method);
    } else if (!constr->isEmpty() && constr->isSolved()) {
        resultType = Types::instantiate(gs, resultType, *constr);
    }
    resultType = Types::replaceSelfType(gs, resultType, args.selfType);

    if (args.block != nullptr) {
        component.sendTp = resultType;
    }
    return result;
}

DispatchResult ClassType::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "classtype");
    vector<TypePtr> empty;
    return dispatchCallSymbol(gs, args, this, symbol, empty);
}

DispatchResult AppliedType::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "appliedType");
    return dispatchCallSymbol(gs, args, this, this->klass, this->targs);
}

TypePtr getMethodArguments(const GlobalState &gs, SymbolRef klass, NameRef name, const vector<TypePtr> &targs) {
    SymbolRef method = klass.data(gs)->findMemberTransitive(gs, name);

    if (!method.exists()) {
        return nullptr;
    }
    const SymbolData data = method.data(gs);

    vector<TypePtr> args;
    args.reserve(data->arguments().size());
    for (const auto &arg : data->arguments()) {
        if (arg.flags.isRepeated) {
            ENFORCE(args.empty(), "getCallArguments with positional and repeated args is not supported: {}",
                    data->toString(gs));
            return Types::arrayOf(gs, Types::resultTypeAsSeenFrom(gs, arg.type, data->owner, klass, targs));
        }
        ENFORCE(!arg.flags.isKeyword, "getCallArguments does not support kwargs: {}", data->toString(gs));
        if (arg.flags.isBlock) {
            continue;
        }
        args.emplace_back(Types::resultTypeAsSeenFrom(gs, arg.type, data->owner, klass, targs));
    }
    return TupleType::build(gs, args);
}

TypePtr ClassType::getCallArguments(const GlobalState &gs, NameRef name) {
    if (isUntyped()) {
        return Types::untyped(gs, untypedBlame());
    }
    return getMethodArguments(gs, symbol, name, vector<TypePtr>{});
}

TypePtr AppliedType::getCallArguments(const GlobalState &gs, NameRef name) {
    return getMethodArguments(gs, klass, name, targs);
}

DispatchResult AliasType::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    Exception::raise("AliasType::dispatchCall");
}

TypePtr AliasType::getCallArguments(const GlobalState &gs, NameRef name) {
    Exception::raise("AliasType::getCallArgumentType");
}

DispatchResult MetaType::dispatchCall(const GlobalState &gs, DispatchArgs args) {
    switch (args.name._id) {
        case Names::new_()._id: {
            auto innerArgs = DispatchArgs{Names::initialize(), args.locs, args.args, wrapped, wrapped, args.block};
            auto original = wrapped->dispatchCall(gs, innerArgs);
            original.returnType = wrapped;
            original.main.sendTp = wrapped;
            return original;
        }
        default:
            auto loc = core::Loc(args.locs.file, args.locs.call);
            if (auto e = gs.beginError(loc, errors::Infer::MetaTypeDispatchCall)) {
                e.setHeader("Call to method `{}` on `{}` mistakes a type for a value", args.name.data(gs)->show(gs),
                            this->wrapped->show(gs));
                if (args.name == core::Names::tripleEq()) {
                    if (auto appliedType = cast_type<AppliedType>(this->wrapped.get())) {
                        e.addErrorSection(
                            ErrorSection("It looks like you're trying to pattern match on a generic", {}));
                        e.replaceWith("Replace with class name", loc, "{}", appliedType->klass.data(gs)->show(gs));
                    }
                }
            }
            return ProxyType::dispatchCall(gs, args);
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
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        res.returnType = Types::untypedUntracked();
    }
} T_untyped;

class T_must : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.empty()) {
            return;
        }
        const auto loc = core::Loc(args.locs.file, args.locs.call);
        if (!args.args[0]->type->isFullyDefined()) {
            if (auto e = gs.beginError(loc, errors::Infer::BareTypeUsage)) {
                e.setHeader("T.must() applied to incomplete type `{}`", args.args[0]->type->show(gs));
            }
            return;
        }
        auto ret = Types::approximateSubtract(gs, args.args[0]->type, Types::nilClass());
        if (ret == args.args[0]->type) {
            if (auto e = gs.beginError(loc, errors::Infer::InvalidCast)) {
                e.setHeader("T.must(): Expected a `T.nilable` type, got: `{}`", args.args[0]->type->show(gs));
                const auto locWithoutTMust = Loc{loc.file(), loc.beginPos() + 7, loc.endPos() - 1};
                e.replaceWith("Remove `T.must`", loc, "{}", locWithoutTMust.source(gs));
            }
        }
        res.returnType = move(ret);
    }
} T_must;

class T_any : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.empty()) {
            return;
        }

        TypePtr ret = Types::bottom();
        auto i = -1;
        for (auto &arg : args.args) {
            i++;
            auto ty = unwrapType(gs, core::Loc(args.locs.file, args.locs.args[i]), arg->type);
            ret = Types::any(gs, ret, ty);
        }

        res.returnType = make_type<MetaType>(move(ret));
    }
} T_any;

class T_all : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.empty()) {
            return;
        }

        TypePtr ret = Types::top();
        auto i = -1;
        for (auto &arg : args.args) {
            i++;
            auto ty = unwrapType(gs, core::Loc(args.locs.file, args.locs.args[i]), arg->type);
            ret = Types::all(gs, ret, ty);
        }

        res.returnType = make_type<MetaType>(move(ret));
    }
} T_all;

class T_revealType : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }

        if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.call), errors::Infer::RevealType)) {
            e.setHeader("Revealed type: `{}`", args.args[0]->type->showWithMoreInfo(gs));
            e.addErrorSection(ErrorSection("From:", args.args[0]->origins2Explanations(gs)));
        }
        res.returnType = args.args[0]->type;
    }
} T_revealType;

class T_nilable : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }

        res.returnType = make_type<MetaType>(Types::any(
            gs, unwrapType(gs, core::Loc(args.locs.file, args.locs.args[0]), args.args[0]->type), Types::nilClass()));
    }
} T_nilable;

class T_proc : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // NOTE: real validation done during infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} T_proc;

class Object_class : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        SymbolRef self = unwrapSymbol(thisType);
        auto singleton = self.data(gs)->lookupSingletonClass(gs);
        if (singleton.exists()) {
            res.returnType = singleton.data(gs)->externalType(gs);
        } else {
            res.returnType = Types::classClass();
        }
    }
} Object_class;

class Class_new : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        SymbolRef self = unwrapSymbol(thisType);

        auto attachedClass = self.data(gs)->attachedClass(gs);
        if (!attachedClass.exists()) {
            if (self == Symbols::Class()) {
                // `Class.new(...)`, but it isn't a specific Class. We know
                // calling .new on a Class will yield some sort of Object
                attachedClass = Symbols::Object();
            } else {
                return;
            }
        }
        auto instanceTy = attachedClass.data(gs)->externalType(gs);
        DispatchArgs innerArgs{Names::initialize(), args.locs, args.args, instanceTy, instanceTy, args.block};
        auto dispatched = instanceTy->dispatchCall(gs, innerArgs);

        for (auto &err : res.main.errors) {
            dispatched.main.errors.emplace_back(std::move(err));
        }
        res.main.errors.clear();
        res.returnType = instanceTy;
        res.main = move(dispatched.main);
        res.main.sendTp = instanceTy;
    }
} Class_new;

class T_Generic_squareBrackets : public IntrinsicMethod {
public:
    // This method is actually special: not only is it called from processBinding in infer, it's
    // also called directly by type_syntax parsing in resolver (because this method checks some
    // invariants of generics that we want to hold even in `typed: false` files).
    //
    // Unfortunately, this means that some errors are double reported (once by resolver, and then
    // again by infer).
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        SymbolRef attachedClass;

        SymbolRef self = unwrapSymbol(thisType);
        attachedClass = self.data(gs)->attachedClass(gs);

        if (!attachedClass.exists()) {
            return;
        }

        if (attachedClass == Symbols::T_Array()) {
            attachedClass = Symbols::Array();
        } else if (attachedClass == Symbols::T_Hash()) {
            attachedClass = Symbols::Hash();
        } else if (attachedClass == Symbols::T_Enumerable()) {
            attachedClass = Symbols::Enumerable();
        } else if (attachedClass == Symbols::T_Enumerator()) {
            attachedClass = Symbols::Enumerator();
        } else if (attachedClass == Symbols::T_Range()) {
            attachedClass = Symbols::Range();
        } else if (attachedClass == Symbols::T_Set()) {
            attachedClass = Symbols::Set();
        }

        auto arity = attachedClass.data(gs)->typeArity(gs);
        if (attachedClass == Symbols::Hash()) {
            arity = 2;
        }
        if (attachedClass.data(gs)->typeMembers().empty()) {
            return;
        }

        if (args.args.size() != arity) {
            if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.call),
                                       errors::Infer::GenericArgumentCountMismatch)) {
                e.setHeader("Wrong number of type parameters for `{}`. Expected: `{}`, got: `{}`",
                            attachedClass.data(gs)->show(gs), arity, args.args.size());
            }
        }

        vector<TypePtr> targs;
        auto it = args.args.begin();
        int i = -1;
        targs.reserve(attachedClass.data(gs)->typeMembers().size());
        for (auto mem : attachedClass.data(gs)->typeMembers()) {
            ++i;

            auto memData = mem.data(gs);

            auto *memType = cast_type<LambdaParam>(memData->resultType.get());
            ENFORCE(memType != nullptr);

            if (memData->isFixed()) {
                // Fixed args are implicitly applied, and won't consume type
                // arguments from the list that's supplied.
                targs.emplace_back(memType->upperBound);
            } else if (it != args.args.end()) {
                auto loc = core::Loc(args.locs.file, args.locs.args[it - args.args.begin()]);
                auto argType = unwrapType(gs, loc, (*it)->type);
                bool validBounds = true;

                // Validate type parameter bounds.
                if (!Types::isSubType(gs, argType, memType->upperBound)) {
                    validBounds = false;
                    if (auto e = gs.beginError(loc, errors::Resolver::GenericTypeParamBoundMismatch)) {
                        auto argStr = argType->show(gs);
                        e.setHeader("`{}` is not a subtype of upper bound of type member `{}`", argStr,
                                    memData->showFullName(gs));
                        e.addErrorLine(memData->loc(), "`{}` is `{}` bounded by `{}` here", memData->showFullName(gs),
                                       "upper", memType->upperBound->show(gs));
                    }
                }

                if (!Types::isSubType(gs, memType->lowerBound, argType)) {
                    validBounds = false;

                    if (auto e = gs.beginError(loc, errors::Resolver::GenericTypeParamBoundMismatch)) {
                        auto argStr = argType->show(gs);
                        e.setHeader("`{}` is not a supertype of lower bound of type member `{}`", argStr,
                                    memData->showFullName(gs));
                        e.addErrorLine(memData->loc(), "`{}` is `{}` bounded by `{}` here", memData->showFullName(gs),
                                       "lower", memType->lowerBound->show(gs));
                    }
                }

                if (validBounds) {
                    targs.emplace_back(argType);
                } else {
                    targs.emplace_back(Types::untypedUntracked());
                }

                ++it;
            } else if (attachedClass == Symbols::Hash() && i == 2) {
                auto tupleArgs = targs;
                targs.emplace_back(TupleType::build(gs, tupleArgs));
            } else {
                targs.emplace_back(Types::untypedUntracked());
            }
        }

        res.returnType = make_type<MetaType>(make_type<AppliedType>(attachedClass, move(targs)));
    }
} T_Generic_squareBrackets;

class SorbetPrivateStatic_sig : public IntrinsicMethod {
public:
    // Forward Sorbet::Private::Static.sig(recv, ...) {...} to recv.sig(...) {...}
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.size() < 1) {
            return;
        }

        auto callLocsReceiver = args.locs.args[0];
        auto callLocsArgs = InlinedVector<LocOffsets, 2>{};
        for (auto loc = args.locs.args.begin() + 1; loc != args.locs.args.end(); ++loc) {
            callLocsArgs.emplace_back(*loc);
        }
        CallLocs callLocs{args.locs.file, args.locs.call, callLocsReceiver, callLocsArgs};

        auto dispatchArgsArgs = InlinedVector<const TypeAndOrigins *, 2>{};
        for (auto arg = args.args.begin() + 1; arg != args.args.end(); ++arg) {
            dispatchArgsArgs.emplace_back(*arg);
        }

        auto recv = args.args[0]->type;
        res = recv->dispatchCall(gs, {core::Names::sig(), callLocs, dispatchArgsArgs, recv, recv, args.block});
    }
} SorbetPrivateStatic_sig;

class Magic_buildHashOrKeywordArgs : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        ENFORCE(args.args.size() % 2 == 0);

        vector<TypePtr> keys;
        vector<TypePtr> values;
        keys.reserve(args.args.size() / 2);
        values.reserve(args.args.size() / 2);
        for (int i = 0; i < args.args.size(); i += 2) {
            auto *key = cast_type<LiteralType>(args.args[i]->type.get());
            if (key == nullptr) {
                res.returnType = Types::hashOfUntyped();
                return;
            }

            keys.emplace_back(args.args[i]->type);
            values.emplace_back(args.args[i + 1]->type);
        }
        res.returnType = make_type<ShapeType>(Types::hashOfUntyped(), move(keys), move(values));
    }
} Magic_buildHashOrKeywordArgs;

class Magic_buildArray : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.empty()) {
            res.returnType = Types::arrayOfUntyped();
            return;
        }
        vector<TypePtr> elems;
        elems.reserve(args.args.size());
        bool isType = absl::c_any_of(args.args, [](auto ty) { return isa_type<MetaType>(ty->type.get()); });
        int i = -1;
        for (auto &elem : args.args) {
            ++i;
            if (isType) {
                elems.emplace_back(unwrapType(gs, core::Loc(args.locs.file, args.locs.args[i]), elem->type));
            } else {
                elems.emplace_back(elem->type);
            }
        }

        auto tuple = TupleType::build(gs, elems);
        if (isType) {
            tuple = make_type<MetaType>(move(tuple));
        }
        res.returnType = move(tuple);
    }
} Magic_buildArray;

class Magic_buildRange : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        ENFORCE(args.args.size() == 3, "Magic_buildRange called with missing arguments");

        auto rangeElemType = Types::dropLiteral(args.args[0]->type);
        auto firstArgIsNil = rangeElemType->isNilClass();
        if (!firstArgIsNil) {
            rangeElemType = Types::dropNil(gs, rangeElemType);
        }
        auto other = Types::dropLiteral(args.args[1]->type);
        auto secondArgIsNil = other->isNilClass();
        if (firstArgIsNil) {
            if (secondArgIsNil) {
                rangeElemType = Types::untypedUntracked();
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
        if (auto *ot = cast_type<OrType>(type.get())) {
            return Types::any(gs, expandArray(gs, ot->left, expandTo), expandArray(gs, ot->right, expandTo));
        }

        auto *tuple = cast_type<TupleType>(type.get());
        if (tuple == nullptr && core::Types::approximate(gs, type, core::TypeConstraint::EmptyFrozenConstraint)
                                    ->derivesFrom(gs, Symbols::Array())) {
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

        return TupleType::build(gs, types);
    }

public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.size() != 3) {
            res.returnType = Types::arrayOfUntyped();
            return;
        }
        auto val = args.args.front()->type;
        auto *beforeLit = cast_type<LiteralType>(args.args[1]->type.get());
        auto *afterLit = cast_type<LiteralType>(args.args[2]->type.get());
        if (!(beforeLit->underlying()->derivesFrom(gs, Symbols::Integer()) &&
              afterLit->underlying()->derivesFrom(gs, Symbols::Integer()))) {
            res.returnType = Types::untypedUntracked();
            return;
        }
        int before = (int)beforeLit->value;
        int after = (int)afterLit->value;
        res.returnType = expandArray(gs, val, before + after);
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
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.size() != 3) {
            return;
        }
        auto &receiver = args.args[0];
        if (receiver->type->isUntyped()) {
            res.returnType = receiver->type;
            return;
        }

        if (!receiver->type->isFullyDefined()) {
            return;
        }

        auto *lit = cast_type<LiteralType>(args.args[1]->type.get());
        if (!lit || !lit->derivesFrom(gs, Symbols::Symbol())) {
            return;
        }
        NameRef fn(gs, (u4)lit->value);
        if (args.args[2]->type->isUntyped()) {
            res.returnType = args.args[2]->type;
            return;
        }
        auto *tuple = cast_type<TupleType>(args.args[2]->type.get());
        if (tuple == nullptr) {
            if (auto e =
                    gs.beginError(core::Loc(args.locs.file, args.locs.args[2]), core::errors::Infer::UntypedSplat)) {
                e.setHeader("Splats are only supported where the size of the array is known statically");
            }
            return;
        }

        InlinedVector<TypeAndOrigins, 2> sendArgStore;
        InlinedVector<const TypeAndOrigins *, 2> sendArgs =
            Magic_callWithSplat::generateSendArgs(tuple, sendArgStore, core::Loc(args.locs.file, args.locs.args[2]));
        InlinedVector<LocOffsets, 2> sendArgLocs(tuple->elems.size(), args.locs.args[2]);
        CallLocs sendLocs{args.locs.file, args.locs.call, args.locs.args[0], sendArgLocs};
        DispatchArgs innerArgs{fn, sendLocs, sendArgs, receiver->type, receiver->type, args.block};
        auto dispatched = receiver->type->dispatchCall(gs, innerArgs);
        for (auto &err : dispatched.main.errors) {
            res.main.errors.emplace_back(std::move(err));
        }
        dispatched.main.errors.clear();

        // TODO: this should merge constrains from `res` and `dispatched` instead
        if ((dispatched.main.constr == nullptr) || dispatched.main.constr->isEmpty()) {
            dispatched.main.constr = move(res.main.constr);
        }
        res.main = move(dispatched.main);

        res.returnType = dispatched.returnType;
        return;
    }
} Magic_callWithSplat;

class Magic_callWithBlock : public IntrinsicMethod {
    friend class Magic_callWithSplatAndBlock;

private:
    static TypePtr typeToProc(const GlobalState &gs, TypePtr blockType, core::FileRef file, LocOffsets callLoc,
                              LocOffsets receiverLoc) {
        auto nonNilBlockType = blockType;
        auto typeIsNilable = false;
        if (Types::isSubType(gs, Types::nilClass(), blockType)) {
            nonNilBlockType = Types::dropNil(gs, blockType);
            typeIsNilable = true;

            if (nonNilBlockType->isBottom()) {
                return Types::nilClass();
            }
        }

        NameRef to_proc = core::Names::toProc();
        InlinedVector<const TypeAndOrigins *, 2> sendArgs;
        InlinedVector<LocOffsets, 2> sendArgLocs;
        CallLocs sendLocs{file, callLoc, receiverLoc, sendArgLocs};
        DispatchArgs innerArgs{to_proc, sendLocs, sendArgs, nonNilBlockType, nonNilBlockType, nullptr};
        auto dispatched = nonNilBlockType->dispatchCall(gs, innerArgs);
        for (auto &err : dispatched.main.errors) {
            gs._error(std::move(err));
        }

        if (typeIsNilable) {
            return Types::any(gs, dispatched.returnType, Types::nilClass());
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

    static void showLocationOfArgDefn(const GlobalState &gs, ErrorBuilder &e, TypePtr blockType,
                                      DispatchComponent &dispatchComp) {
        if (!dispatchComp.method.exists()) {
            return;
        }

        if (dispatchComp.method.data(gs)->isClassOrModule()) {
            return;
        }

        const auto &methodArgs = dispatchComp.method.data(gs)->arguments();
        ENFORCE(!methodArgs.empty());
        const auto &bspec = methodArgs.back();
        ENFORCE(bspec.flags.isBlock);
        e.addErrorSection(ErrorSection({
            ErrorLine::from(bspec.loc, "Method `{}` has specified `{}` as `{}`", dispatchComp.method.data(gs)->show(gs),
                            bspec.argumentName(gs), blockType->show(gs)),
        }));
    }

    static void simulateCall(const GlobalState &gs, const TypeAndOrigins *receiver, DispatchArgs innerArgs,
                             shared_ptr<SendAndBlockLink> link, TypePtr passedInBlockType, Loc callLoc, Loc blockLoc,
                             DispatchResult &res) {
        auto dispatched = receiver->type->dispatchCall(gs, innerArgs);
        for (auto &err : dispatched.main.errors) {
            res.main.errors.emplace_back(std::move(err));
        }
        dispatched.main.errors.clear();
        // We use isSubTypeUnderConstraint here with a TypeConstraint, so that we discover the correct generic bounds
        // as we do the subtyping check.
        auto &constr = dispatched.main.constr;
        auto &blockPreType = dispatched.main.blockPreType;
        if (blockPreType && !Types::isSubTypeUnderConstraint(gs, *constr, passedInBlockType, blockPreType,
                                                             UntypedMode::AlwaysCompatible)) {
            ClassType *passedInProcClass = cast_type<ClassType>(passedInBlockType.get());
            auto nonNilableBlockType = Types::dropNil(gs, blockPreType);
            if (passedInProcClass && passedInProcClass->symbol == Symbols::Proc() &&
                Types::isSubType(gs, nonNilableBlockType, passedInBlockType)) {
                // If a block of unknown arity is passed in, but the function was declared with a known arity,
                // raise an error in strict mode.
                // This could occur, for example, when using Method#to_proc, since we type it as returning a `Proc`.
                if (auto e = gs.beginError(blockLoc, errors::Infer::ProcArityUnknown)) {
                    e.setHeader("Cannot use a `{}` with unknown arity as a `{}`", "Proc", blockPreType->show(gs));
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
                    passedInBlockType = make_type<core::AppliedType>(procWithCorrectArity, targs);
                }
            } else if (auto e = gs.beginError(blockLoc, errors::Infer::MethodArgumentMismatch)) {
                e.setHeader("Expected `{}` but found `{}` for block argument", blockPreType->show(gs),
                            passedInBlockType->show(gs));
                if (!dispatched.secondary) {
                    Magic_callWithBlock::showLocationOfArgDefn(gs, e, blockPreType, dispatched.main);
                }
            }
        }

        {
            auto it = &dispatched;
            while (it != nullptr) {
                if (it->main.method.exists() && !it->main.method.data(gs)->isClassOrModule()) {
                    const auto &methodArgs = it->main.method.data(gs)->arguments();
                    ENFORCE(!methodArgs.empty());
                    const auto &bspec = methodArgs.back();
                    ENFORCE(bspec.flags.isBlock);

                    auto bspecType = bspec.type;
                    if (bspecType) {
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
                if (auto e = gs.beginError(callLoc, errors::Infer::GenericMethodConstaintUnsolved)) {
                    e.setHeader("Could not find valid instantiation of type parameters");
                }
                res.returnType = core::Types::untypedUntracked();
            }

            if (!constr->isEmpty() && constr->isSolved()) {
                dispatched.returnType = Types::instantiate(gs, dispatched.returnType, *(constr));
            }
        }
        res.returnType = dispatched.returnType;
    }

public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] is the block
        // args[3...] are the remaining arguements
        // equivalent to (args[0]).args[1](*args[3..], &args[2])

        if (args.args.size() < 3) {
            return;
        }
        auto &receiver = args.args[0];
        if (receiver->type->isUntyped()) {
            res.returnType = receiver->type;
            return;
        }

        if (!receiver->type->isFullyDefined()) {
            return;
        }

        if (core::cast_type<core::TypeVar>(args.args[2]->type.get())) {
            if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.args[2]),
                                       core::errors::Infer::GenericPassedAsBlock)) {
                e.setHeader("Passing generics as block arguments is not supported");
            }
            return;
        }

        auto *lit = cast_type<LiteralType>(args.args[1]->type.get());
        if (!lit || !lit->derivesFrom(gs, Symbols::Symbol())) {
            return;
        }
        NameRef fn(gs, (u4)lit->value);

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
        CallLocs sendLocs{args.locs.file, args.locs.call, args.locs.args[0], sendArgLocs};

        TypePtr finalBlockType =
            Magic_callWithBlock::typeToProc(gs, args.args[2]->type, args.locs.file, args.locs.call, args.locs.args[2]);
        std::optional<int> blockArity = Magic_callWithBlock::getArityForBlock(finalBlockType);
        auto link = make_shared<core::SendAndBlockLink>(fn, Magic_callWithBlock::argInfoByArity(blockArity), -1);
        res.main.constr = make_unique<TypeConstraint>();

        DispatchArgs innerArgs{fn, sendLocs, sendArgs, receiver->type, receiver->type, link};

        Magic_callWithBlock::simulateCall(gs, receiver, innerArgs, link, finalBlockType,
                                          core::Loc(args.locs.file, args.locs.args[2]),
                                          core::Loc(args.locs.file, args.locs.call), res);
    }
} Magic_callWithBlock;

class Magic_callWithSplatAndBlock : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] are the splat arguments
        // args[3] is the block

        if (args.args.size() != 4) {
            return;
        }
        auto &receiver = args.args[0];
        if (receiver->type->isUntyped()) {
            res.returnType = receiver->type;
            return;
        }

        if (!receiver->type->isFullyDefined()) {
            return;
        }

        auto *lit = cast_type<LiteralType>(args.args[1]->type.get());
        if (!lit || !lit->derivesFrom(gs, Symbols::Symbol())) {
            return;
        }
        NameRef fn(gs, (u4)lit->value);

        if (args.args[2]->type->isUntyped()) {
            res.returnType = args.args[2]->type;
            return;
        }
        auto *tuple = cast_type<TupleType>(args.args[2]->type.get());
        if (tuple == nullptr) {
            if (auto e =
                    gs.beginError(core::Loc(args.locs.file, args.locs.args[2]), core::errors::Infer::UntypedSplat)) {
                e.setHeader("Splats are only supported where the size of the array is known statically");
            }
            return;
        }

        if (core::cast_type<core::TypeVar>(args.args[3]->type.get())) {
            if (auto e = gs.beginError(core::Loc(args.locs.file, args.locs.args[3]),
                                       core::errors::Infer::GenericPassedAsBlock)) {
                e.setHeader("Passing generics as block arguments is not supported");
            }
            return;
        }

        InlinedVector<TypeAndOrigins, 2> sendArgStore;
        InlinedVector<const TypeAndOrigins *, 2> sendArgs =
            Magic_callWithSplat::generateSendArgs(tuple, sendArgStore, core::Loc(args.locs.file, args.locs.args[2]));
        InlinedVector<LocOffsets, 2> sendArgLocs(tuple->elems.size(), args.locs.args[2]);
        CallLocs sendLocs{args.locs.file, args.locs.call, args.locs.args[0], sendArgLocs};

        TypePtr finalBlockType =
            Magic_callWithBlock::typeToProc(gs, args.args[3]->type, args.locs.file, args.locs.call, args.locs.args[3]);
        std::optional<int> blockArity = Magic_callWithBlock::getArityForBlock(finalBlockType);
        auto link = make_shared<core::SendAndBlockLink>(fn, Magic_callWithBlock::argInfoByArity(blockArity), -1);
        res.main.constr = make_unique<TypeConstraint>();

        DispatchArgs innerArgs{fn, sendLocs, sendArgs, receiver->type, receiver->type, link};

        Magic_callWithBlock::simulateCall(gs, receiver, innerArgs, link, finalBlockType,
                                          core::Loc(args.locs.file, args.locs.args[3]),
                                          core::Loc(args.locs.file, args.locs.call), res);
    }
} Magic_callWithSplatAndBlock;

class Magic_suggestUntypedConstantType : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        ENFORCE(args.args.size() == 1);
        auto ty = core::Types::widen(gs, args.args.front()->type);
        auto loc = core::Loc(args.locs.file, args.locs.args[0]);
        if (auto e = gs.beginError(loc, core::errors::Infer::UntypedConstantSuggestion)) {
            e.setHeader("Constants must have type annotations with `{}` when specifying `{}`", "T.let",
                        "# typed: strict");
            if (!ty->isUntyped() && loc.exists()) {
                e.replaceWith(fmt::format("Initialize as `{}`", ty->show(gs)), loc, "T.let({}, {})", loc.source(gs),
                              ty->show(gs));
            }
        }
        res.returnType = move(ty);
    }
} Magic_suggestUntypedConstantType;

/**
 * This is a special version of `new` that will return `T.attached_class`
 * instead.
 */
class Magic_selfNew : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // args[0] is the Class to create an instance of
        // args[1..] are the arguments to the constructor

        if (args.args.empty()) {
            res.returnType = core::Types::untypedUntracked();
            return;
        }

        auto selfTy = args.args[0]->type;
        SymbolRef self = unwrapSymbol(selfTy.get());

        InlinedVector<const TypeAndOrigins *, 2> sendArgStore;
        InlinedVector<LocOffsets, 2> sendArgLocs;
        for (int i = 1; i < args.args.size(); ++i) {
            sendArgStore.emplace_back(args.args[i]);
            sendArgLocs.emplace_back(args.locs.args[i]);
        }
        CallLocs sendLocs{args.locs.file, args.locs.call, args.locs.args[0], sendArgLocs};

        TypePtr returnTy;
        DispatchResult dispatched;
        if (!self.data(gs)->isSingletonClass(gs)) {
            // In the case that `self` is not a singleton class, we know that
            // this was a call to `new` outside of a self context. Dispatch to
            // an instance method named new, and see what happens.
            DispatchArgs innerArgs{Names::new_(), sendLocs, sendArgStore, selfTy, selfTy, args.block};
            dispatched = selfTy->dispatchCall(gs, innerArgs);
            returnTy = dispatched.returnType;
        } else {
            // Otherwise, we know that this is the proper new intrinsic, and we
            // should be returning something of type `T.attached_class`
            auto attachedClass = self.data(gs)->findMember(gs, core::Names::Constants::AttachedClass());

            // AttachedClass will only be missing on `T.untyped`
            ENFORCE(attachedClass.exists());

            auto instanceTy = self.data(gs)->attachedClass(gs).data(gs)->externalType(gs);
            DispatchArgs innerArgs{Names::initialize(), sendLocs, sendArgStore, instanceTy, instanceTy, args.block};
            dispatched = instanceTy->dispatchCall(gs, innerArgs);

            // The return type from dispatched is ignored, and we return
            // `T.attached_class` instead.
            returnTy = make_type<SelfTypeParam>(attachedClass);
        }

        for (auto &err : res.main.errors) {
            dispatched.main.errors.emplace_back(std::move(err));
        }
        res.main.errors.clear();
        res.main = move(dispatched.main);
        res.returnType = returnTy;
        res.main.sendTp = returnTy;
    }
} Magic_selfNew;

class DeclBuilderForProcs_void : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // NOTE: real validation done in infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} DeclBuilderForProcs_void;

class DeclBuilderForProcs_returns : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // NOTE: real validation done in infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} DeclBuilderForProcs_returns;

class DeclBuilderForProcs_params : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // NOTE: real validation done in infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} DeclBuilderForProcs_params;

class DeclBuilderForProcs_bind : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // NOTE: real validation done in infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} DeclBuilderForProcs_bind;

class Tuple_squareBrackets : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        auto *tuple = cast_type<TupleType>(thisType);
        ENFORCE(tuple);
        LiteralType *lit = nullptr;
        if (args.args.size() == 1) {
            lit = cast_type<LiteralType>(args.args.front()->type.get());
        }
        if (!lit || !lit->underlying()->derivesFrom(gs, Symbols::Integer())) {
            return;
        }

        auto idx = lit->value;
        if (idx < 0) {
            idx = tuple->elems.size() + idx;
        }
        if (idx >= tuple->elems.size()) {
            res.returnType = Types::nilClass();
        } else {
            res.returnType = tuple->elems[idx];
        }
    }
} Tuple_squareBrackets;

class Tuple_last : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        auto *tuple = cast_type<TupleType>(thisType);
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
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        auto *tuple = cast_type<TupleType>(thisType);
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
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        auto *tuple = cast_type<TupleType>(thisType);
        ENFORCE(tuple);

        if (!args.args.empty()) {
            return;
        }
        if (tuple->elems.empty()) {
            res.returnType = Types::nilClass();
        } else {
            res.returnType = tuple->elementType();
        }
    }
} Tuple_minMax;

class Tuple_to_a : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        res.returnType = args.selfType;
    }
} Tuple_to_a;

class Tuple_concat : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        vector<TypePtr> elems;
        auto *tuple = cast_type<TupleType>(thisType);
        ENFORCE(tuple);
        elems = tuple->elems;
        for (auto elem : args.args) {
            if (auto *tuple = cast_type<TupleType>(elem->type.get())) {
                elems.insert(elems.end(), tuple->elems.begin(), tuple->elems.end());
            } else {
                return;
            }
        }
        res.returnType = TupleType::build(gs, std::move(elems));
    }
} Tuple_concat;

class Shape_merge : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        auto *shape = cast_type<ShapeType>(thisType);
        ENFORCE(shape);
        ShapeType *rhs = nullptr;
        if (!args.args.empty()) {
            rhs = cast_type<ShapeType>(args.args.front()->type.get());
        }
        if (rhs == nullptr || args.block != nullptr || args.args.size() > 1) {
            return;
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

        res.returnType = make_type<ShapeType>(Types::hashOfUntyped(), std::move(keys), std::move(values));
    }
} Shape_merge;

class Array_flatten : public IntrinsicMethod {
    // Flattens a (nested) array all way down to its (inner) element type, stopping if we hit the depth limit first.
    static TypePtr recursivelyFlattenArrays(const GlobalState &gs, const TypePtr &type, const int64_t depth) {
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
                result = Types::any(gs, recursivelyFlattenArrays(gs, o->left, newDepth),
                                    recursivelyFlattenArrays(gs, o->right, newDepth));
            },

            [&](AppliedType *a) {
                if (a->klass != Symbols::Array()) {
                    result = type;
                    return;
                }
                ENFORCE(a->targs.size() == 1);
                result = recursivelyFlattenArrays(gs, a->targs.front(), newDepth);
            },

            [&](TupleType *t) { result = recursivelyFlattenArrays(gs, t->elementType(), newDepth); },

            [&](Type *t) { result = std::move(type); });
        return result;
    }

public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // Unwrap the array one time to get the element type (we'll rewrap it down at the bottom)
        TypePtr element;
        if (auto *ap = cast_type<AppliedType>(thisType)) {
            ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(gs)->derivesFrom(gs, Symbols::Array()));
            ENFORCE(!ap->targs.empty());
            element = ap->targs.front();
        } else if (auto *tuple = cast_type<TupleType>(thisType)) {
            element = tuple->elementType();
        } else {
            ENFORCE(false, "Array#flatten on unexpected type: {}", args.selfType->show(gs));
        }

        int64_t depth;
        if (args.args.size() == 1) {
            auto argTyp = args.args[0]->type;
            ENFORCE(args.locs.args.size() == 1, "Mismatch between args.size() and args.locs.args.size(): {}",
                    args.locs.args.size());
            auto argLoc = args.locs.args[0];

            auto lt = cast_type<LiteralType>(argTyp.get());
            if (!lt) {
                if (auto e =
                        gs.beginError(core::Loc(args.locs.file, argLoc), core::errors::Infer::ExpectedLiteralType)) {
                    e.setHeader("You must pass an Integer literal to specify a depth with Array#flatten");
                }
                return;
            }
            ENFORCE(lt->literalKind == LiteralType::LiteralTypeKind::Integer, "depth arg must be an Integer literal");

            if (lt->value >= 0) {
                depth = lt->value;
            } else {
                // Negative values behave like no depth was given
                depth = INT64_MAX;
            }
        } else if (args.args.size() == 0) {
            depth = INT64_MAX;
        } else {
            // If our arity is off, then calls.cc will report an error due to mismatch with the RBI elsewhere, so we
            // don't need to do anything special here
            return;
        }

        res.returnType = Types::arrayOf(gs, recursivelyFlattenArrays(gs, element, depth));
    }
} Array_flatten;

class Array_product : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        vector<TypePtr> unwrappedElems;
        unwrappedElems.reserve(args.args.size() + 1);

        if (auto *ap = cast_type<AppliedType>(thisType)) {
            ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(gs)->derivesFrom(gs, Symbols::Array()));
            ENFORCE(!ap->targs.empty());
            unwrappedElems.emplace_back(ap->targs.front());
        } else if (auto *tuple = cast_type<TupleType>(thisType)) {
            unwrappedElems.emplace_back(tuple->elementType());
        } else {
            // We will have only dispatched to this intrinsic when we knew the receiver.
            // Did we register this intrinsic on the wrong symbol?
            ENFORCE(false, "Array#product on unexpected receiver type: {}", args.selfType->show(gs));
            res.returnType = Types::untypedUntracked();
            return;
        }

        for (auto arg : args.args) {
            auto argTyp = arg->type;
            if (auto *ap = cast_type<AppliedType>(argTyp.get())) {
                ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(gs)->derivesFrom(gs, Symbols::Array()));
                ENFORCE(!ap->targs.empty());
                unwrappedElems.emplace_back(ap->targs.front());
            } else if (auto *tuple = cast_type<TupleType>(argTyp.get())) {
                unwrappedElems.emplace_back(tuple->elementType());
            } else {
                // Arg type didn't match; we already reported an error for the arg type; just return untyped to recover.
                res.returnType = Types::untypedUntracked();
                return;
            }
        }

        res.returnType = Types::arrayOf(gs, TupleType::build(gs, unwrappedElems));
    }
} Array_product;

class Array_compact : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        TypePtr element;
        if (auto *ap = cast_type<AppliedType>(thisType)) {
            ENFORCE(ap->klass == Symbols::Array() || ap->klass.data(gs)->derivesFrom(gs, Symbols::Array()));
            ENFORCE(!ap->targs.empty());
            element = ap->targs.front();
        } else if (auto *tuple = cast_type<TupleType>(thisType)) {
            element = tuple->elementType();
        } else {
            ENFORCE(false, "Array#compact on unexpected type: {}", args.selfType->show(gs));
        }
        auto ret = Types::approximateSubtract(gs, element, Types::nilClass());
        res.returnType = Types::arrayOf(gs, ret);
    }
} Array_compact;

class Kernel_proc : public IntrinsicMethod {
public:
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.block == nullptr) {
            return;
        }

        std::optional<int> numberOfPositionalBlockParams = args.block->fixedArity();
        if (!numberOfPositionalBlockParams || *numberOfPositionalBlockParams > core::Symbols::MAX_PROC_ARITY) {
            res.returnType = core::Types::procClass();
            return;
        }
        vector<core::TypePtr> targs(*numberOfPositionalBlockParams + 1, core::Types::untypedUntracked());
        auto procClass = core::Symbols::Proc(*numberOfPositionalBlockParams);
        res.returnType = make_type<core::AppliedType>(procClass, move(targs));
    }
} Kernel_proc;

class Enumerable_toH : public IntrinsicMethod {
public:
    // Forward Enumerable.to_h to RubyType.enumerable_to_h[self]
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        auto hash = make_type<ClassType>(core::Symbols::Sorbet_Private_Static().data(gs)->lookupSingletonClass(gs));
        InlinedVector<LocOffsets, 2> argLocs{args.locs.receiver};
        CallLocs locs{
            args.locs.file,
            args.locs.call,
            args.locs.call,
            argLocs,
        };
        TypeAndOrigins myType{args.selfType, {core::Loc(args.locs.file, args.locs.receiver)}};
        InlinedVector<const TypeAndOrigins *, 2> innerArgs{&myType};

        DispatchArgs dispatch{
            core::Names::enumerableToH(), locs, innerArgs, hash, hash, nullptr,
        };
        auto dispatched = hash->dispatchCall(gs, dispatch);
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
    void apply(const GlobalState &gs, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }
        auto rhs = args.args[0]->type;
        if (rhs->isUntyped()) {
            res.returnType = rhs;
            return;
        }
        auto rc = Types::getRepresentedClass(gs, thisType);
        // in most cases, thisType is T.class_of(rc). see test/testdata/class_not_class_of.rb for an edge case.
        if (rc == core::Symbols::noSymbol()) {
            res.returnType = Types::Boolean();
            return;
        }
        auto lhs = rc.data(gs)->externalType(gs);
        ENFORCE(!lhs->isUntyped(), "lhs of Module.=== must be typed");
        if (Types::isSubType(gs, rhs, lhs)) {
            res.returnType = Types::trueClass();
            return;
        }
        if (Types::glb(gs, rhs, lhs)->isBottom()) {
            res.returnType = Types::falseClass();
            return;
        }
        res.returnType = Types::Boolean();
    }
} Module_tripleEq;

} // namespace

const vector<Intrinsic> intrinsicMethods{
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::untyped(), &T_untyped},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::must(), &T_must},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::all(), &T_all},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::any(), &T_any},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::nilable(), &T_nilable},
    {Symbols::T(), Intrinsic::Kind::Singleton, Names::revealType(), &T_revealType},

    {Symbols::T(), Intrinsic::Kind::Singleton, Names::proc(), &T_proc},

    {Symbols::T_Generic(), Intrinsic::Kind::Instance, Names::squareBrackets(), &T_Generic_squareBrackets},

    {Symbols::T_Array(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Hash(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Enumerable(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Enumerator(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Range(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},
    {Symbols::T_Set(), Intrinsic::Kind::Singleton, Names::squareBrackets(), &T_Generic_squareBrackets},

    {Symbols::Object(), Intrinsic::Kind::Instance, Names::class_(), &Object_class},
    {Symbols::Object(), Intrinsic::Kind::Instance, Names::singletonClass(), &Object_class},

    {Symbols::Class(), Intrinsic::Kind::Instance, Names::new_(), &Class_new},

    {Symbols::Sorbet_Private_Static(), Intrinsic::Kind::Singleton, Names::sig(), &SorbetPrivateStatic_sig},

    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::buildHash(), &Magic_buildHashOrKeywordArgs},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::buildKeywordArgs(), &Magic_buildHashOrKeywordArgs},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::buildArray(), &Magic_buildArray},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::buildRange(), &Magic_buildRange},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::expandSplat(), &Magic_expandSplat},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::callWithSplat(), &Magic_callWithSplat},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::callWithBlock(), &Magic_callWithBlock},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::callWithSplatAndBlock(), &Magic_callWithSplatAndBlock},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::suggestType(), &Magic_suggestUntypedConstantType},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::selfNew(), &Magic_selfNew},

    {Symbols::DeclBuilderForProcsSingleton(), Intrinsic::Kind::Instance, Names::void_(), &DeclBuilderForProcs_void},
    {Symbols::DeclBuilderForProcsSingleton(), Intrinsic::Kind::Instance, Names::returns(),
     &DeclBuilderForProcs_returns},
    {Symbols::DeclBuilderForProcsSingleton(), Intrinsic::Kind::Instance, Names::params(), &DeclBuilderForProcs_params},
    {Symbols::DeclBuilderForProcsSingleton(), Intrinsic::Kind::Instance, Names::bind(), &DeclBuilderForProcs_bind},

    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::squareBrackets(), &Tuple_squareBrackets},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::first(), &Tuple_first},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::last(), &Tuple_last},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::min(), &Tuple_minMax},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::max(), &Tuple_minMax},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::toA(), &Tuple_to_a},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::concat(), &Tuple_concat},

    {Symbols::Shape(), Intrinsic::Kind::Instance, Names::merge(), &Shape_merge},

    {Symbols::Array(), Intrinsic::Kind::Instance, Names::flatten(), &Array_flatten},
    {Symbols::Array(), Intrinsic::Kind::Instance, Names::product(), &Array_product},
    {Symbols::Array(), Intrinsic::Kind::Instance, Names::compact(), &Array_compact},

    {Symbols::Kernel(), Intrinsic::Kind::Instance, Names::proc(), &Kernel_proc},
    {Symbols::Kernel(), Intrinsic::Kind::Instance, Names::lambda(), &Kernel_proc},

    {Symbols::Enumerable(), Intrinsic::Kind::Instance, Names::toH(), &Enumerable_toH},

    {Symbols::Module(), Intrinsic::Kind::Instance, Names::tripleEq(), &Module_tripleEq},
};

} // namespace sorbet::core
