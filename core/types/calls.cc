#include "absl/strings/match.h"
#include "common/common.h"
#include "common/typecase.h"
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
    DispatchResult ret{Types::any(ctx, leftRet.returnType, rightRet.returnType), move(leftRet.main),
                       make_unique<DispatchResult>(move(rightRet)), DispatchResult::Combinator::OR};
    return ret;
}

TypePtr OrType::getCallArguments(Context ctx, NameRef name) {
    auto largs = left->getCallArguments(ctx, name);
    auto rargs = right->getCallArguments(ctx, name);
    if (!largs) {
        largs = Types::untypedUntracked();
    }
    if (!rargs) {
        rargs = Types::untypedUntracked();
    }
    return Types::glb(ctx, largs, rargs);
}

DispatchResult TypeVar::dispatchCall(Context ctx, DispatchArgs args) {
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

DispatchResult AndType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "andtype");
    auto leftRet = left->dispatchCall(ctx, args);
    auto rightRet = right->dispatchCall(ctx, args);

    // If either side is missing the method, dispatch to the other.
    auto leftOk = allComponentsPresent(leftRet);
    auto rightOk = allComponentsPresent(rightRet);
    if (leftOk && !rightOk) {
        return leftRet;
    }
    if (rightOk && !leftOk) {
        return rightRet;
    }
    DispatchResult ret{Types::all(ctx, leftRet.returnType, rightRet.returnType), move(leftRet.main),
                       make_unique<DispatchResult>(move(rightRet)),

                       DispatchResult::Combinator::AND};

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
        DispatchComponent comp{args.selfType, method, {}, nullptr, nullptr, nullptr, ArgInfo{}, nullptr};
        DispatchResult res{nullptr, std::move(comp)};
        method.data(ctx)->intrinsic->apply(ctx, args, this, res);
        if (res.returnType != nullptr) {
            return res;
        }
    }
    return ProxyType::dispatchCall(ctx, args);
}

DispatchResult TupleType::dispatchCall(Context ctx, DispatchArgs args) {
    categoryCounterInc("dispatch_call", "tupletype");
    auto method = Symbols::Tuple().data(ctx)->findMember(ctx, args.name);
    if (method.exists() && method.data(ctx)->intrinsic != nullptr) {
        DispatchComponent comp{args.selfType, method, {}, nullptr, nullptr, nullptr, ArgInfo{}, nullptr};
        DispatchResult res{nullptr, std::move(comp)};
        method.data(ctx)->intrinsic->apply(ctx, args, this, res);
        if (res.returnType != nullptr) {
            return res;
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
                               SymbolRef method, const TypeAndOrigins &argTpe, const ArgInfo &argSym,
                               const TypePtr &selfType, vector<TypePtr> &targs, Loc loc, bool mayBeSetter = false) {
    TypePtr expectedType = Types::resultTypeAsSeenFrom(ctx, argSym.type, method.data(ctx)->owner, inClass, targs);
    if (!expectedType) {
        expectedType = Types::untyped(ctx, method);
    }

    expectedType = Types::replaceSelfType(ctx, expectedType, selfType);

    if (Types::isSubTypeUnderConstraint(ctx, constr, true, argTpe.type, expectedType)) {
        return nullptr;
    }
    if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentMismatch)) {
        if (mayBeSetter && isSetter(ctx, method.data(ctx)->name)) {
            e.setHeader("Assigning a value to `{}` that does not match expected type `{}`", argSym.argumentName(ctx),
                        expectedType->show(ctx));
        } else {
            e.setHeader("Expected `{}` but found `{}` for argument `{}`", expectedType->show(ctx),
                        argTpe.type->show(ctx), argSym.argumentName(ctx));
            e.addErrorSection(ErrorSection({
                ErrorLine::from(argSym.loc, "Method `{}` has specified `{}` as `{}`", method.data(ctx)->show(ctx),
                                argSym.argumentName(ctx), expectedType->show(ctx)),
            }));
        }
        e.addErrorSection(
            ErrorSection("Got " + argTpe.type->show(ctx) + " originating from:", argTpe.origins2Explanations(ctx)));
        auto withoutNil = Types::approximateSubtract(ctx, argTpe.type, Types::nilClass());
        if (!withoutNil->isBottom() && Types::isSubTypeUnderConstraint(ctx, constr, true, withoutNil, expectedType)) {
            if (loc.exists()) {
                e.replaceWith("Wrap in `T.must`", loc, "T.must({})", loc.source(ctx));
            }
        }
        return e.build();
    }
    return nullptr;
}

unique_ptr<Error> missingArg(Context ctx, Loc callLoc, Loc receiverLoc, SymbolRef method, const ArgInfo &arg) {
    if (auto e = ctx.state.beginError(callLoc, errors::Infer::MethodArgumentCountMismatch)) {
        e.setHeader("Missing required keyword argument `{}` for method `{}`", arg.name.show(ctx),
                    method.data(ctx)->show(ctx));
        return e.build();
    }
    return nullptr;
}
}; // namespace

int getArity(Context ctx, SymbolRef method) {
    ENFORCE(!method.data(ctx)->arguments().empty(), "Every method should have at least a block arg.");
    ENFORCE(method.data(ctx)->arguments().back().flags.isBlock, "Last arg should be the block arg.");

    // Don't count the block arg in the arity
    return method.data(ctx)->arguments().size() - 1;
}

// Guess overload. The way we guess is only arity based - we will return the overload that has the smallest number of
// arguments that is >= args.size()
SymbolRef guessOverload(Context ctx, SymbolRef inClass, SymbolRef primary,
                        InlinedVector<const TypeAndOrigins *, 2> &args, const TypePtr &fullType, vector<TypePtr> &targs,
                        bool hasBlock) {
    counterInc("calls.overloaded_invocations");
    ENFORCE(ctx.permitOverloadDefinitions(primary.data(ctx)->loc().file()), "overload not permitted here");
    SymbolRef fallback = primary;
    vector<SymbolRef> allCandidates;

    allCandidates.emplace_back(primary);
    { // create candidates and sort them by number of arguments(stable by symbol id)
        int i = 0;
        SymbolRef current = primary;
        while (current.data(ctx)->isOverloaded()) {
            i++;
            NameRef overloadName = ctx.state.lookupNameUnique(UniqueNameKind::Overload, primary.data(ctx)->name, i);
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

                auto argType = Types::resultTypeAsSeenFrom(ctx, candidate.data(ctx)->arguments()[i].type,
                                                           candidate.data(ctx)->owner, inClass, targs);
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
            const auto &args = candidate.data(ctx)->arguments();
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
        if (classType->symbol.data(ctx)->derivesFrom(ctx, core::Symbols::OpusEnum())) {
            // Opus::Enum instances are allowed to stand for themselves in type syntax positions.
            // See the note in type_syntax.cc regarding Opus::Enum.
            return tp;
        }

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
    for (const auto &arg : method.data(ctx)->arguments()) {
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

bool extendsTHelpers(core::Context ctx, core::SymbolRef enclosingClass) {
    ENFORCE(enclosingClass.exists());
    auto enclosingSingletonClass = enclosingClass.data(ctx)->lookupSingletonClass(ctx);
    ENFORCE(enclosingSingletonClass.exists());
    return enclosingSingletonClass.data(ctx)->derivesFrom(ctx, core::Symbols::T_Helpers());
}

/**
 * Make an autocorrection for adding `extend T::Helpers`, when needed.
 */
optional<core::AutocorrectSuggestion> maybeSuggestExtendTHelpers(core::Context ctx, const Type *thisType,
                                                                 const Loc &call) {
    auto *classType = cast_type<ClassType>(thisType);
    if (classType == nullptr) {
        return nullopt;
    }

    auto enclosingClass = classType->symbol.data(ctx)->topAttachedClass(ctx);
    if (extendsTHelpers(ctx, enclosingClass)) {
        // No need to suggest here, because it already has 'extend T::Sig'
        return nullopt;
    }

    auto inFileOfMethod = [&](const auto &loc) { return loc.file() == call.file(); };
    auto classLocs = enclosingClass.data(ctx)->locs();
    auto classLoc = absl::c_find_if(classLocs, inFileOfMethod);

    if (classLoc == classLocs.end()) {
        // Couldn't a loc for the enclosing class in this file, give up.
        return nullopt;
    }

    auto [classStart, classEnd] = classLoc->position(ctx);

    core::Loc::Detail thisLineStart = {classStart.line, 1};
    core::Loc thisLineLoc = core::Loc::fromDetails(ctx, classLoc->file(), thisLineStart, thisLineStart);
    auto [_, thisLinePadding] = thisLineLoc.findStartOfLine(ctx);

    ENFORCE(classStart.line + 1 <= classLoc->file().data(ctx).lineBreaks().size());
    core::Loc::Detail nextLineStart = {classStart.line + 1, 1};
    core::Loc nextLineLoc = core::Loc::fromDetails(ctx, classLoc->file(), nextLineStart, nextLineStart);
    auto [replacementLoc, nextLinePadding] = nextLineLoc.findStartOfLine(ctx);

    // Preserve the indentation of the line below us.
    string prefix(max(thisLinePadding + 2, nextLinePadding), ' ');
    return core::AutocorrectSuggestion{
        "Add `extend T::Helpers`",
        {core::AutocorrectSuggestion::Edit{nextLineLoc, fmt::format("{}extend T::Helpers\n", prefix)}}};
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
                    result.main.errors.emplace_back(e.build());
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

                // catch the special case of `interface!`, `abstract!`, `final!`, or `sealed!` and
                // suggest adding `extend T::Helpers`.
                if (args.name == core::Names::declareInterface() || args.name == core::Names::declareAbstract() ||
                    args.name == core::Names::declareFinal() || args.name == core::Names::declareSealed()) {
                    if (auto suggestion = maybeSuggestExtendTHelpers(ctx, thisType, args.locs.call)) {
                        e.addAutocorrect(std::move(*suggestion));
                    }
                }
            }
            if (args.fullType.get() != thisType && symbol == Symbols::NilClass()) {
                e.replaceWith("Wrap in `T.must`", args.locs.receiver, "T.must({})", args.locs.receiver.source(ctx));
            } else {
                if (symbol.data(ctx)->isClassModule()) {
                    auto objMeth = core::Symbols::Object().data(ctx)->findMemberTransitive(ctx, args.name);
                    if (objMeth.exists() && objMeth.data(ctx)->owner.data(ctx)->isClassModule()) {
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
                        auto possibleSymbol = alternative.symbol.data(ctx);
                        if (!possibleSymbol->isClass() && !possibleSymbol->isMethod()) {
                            continue;
                        }

                        auto suggestedName = possibleSymbol->isClass() ? alternative.symbol.show(ctx) + ".new"
                                                                       : alternative.symbol.show(ctx);

                        bool addedAutocorrect = false;
                        if (possibleSymbol->isClass()) {
                            const auto replacement = possibleSymbol->name.show(ctx);
                            const auto loc = args.locs.call;
                            const auto toReplace = args.name.toString(ctx);
                            // This is a bit hacky but the loc corresponding to the send isn't available here and until
                            // it is, this verifies that the methodLoc below exists.
                            if (absl::StartsWith(loc.source(ctx), toReplace)) {
                                const auto methodLoc =
                                    Loc{loc.file(), loc.beginPos(), (u4)(loc.beginPos() + toReplace.length())};
                                e.replaceWith(fmt::format("Replace with `{}.new`", replacement), methodLoc, "{}.new",
                                              replacement);
                                addedAutocorrect = true;
                            }
                        } else {
                            const auto replacement = possibleSymbol->name.toString(ctx);
                            const auto toReplace = args.name.toString(ctx);
                            if (replacement != toReplace) {
                                const auto loc = args.locs.receiver;
                                // See comment above.
                                if (absl::StartsWith(args.locs.call.source(ctx),
                                                     fmt::format("{}.{}", loc.source(ctx), toReplace))) {
                                    const auto methodLoc =
                                        Loc{loc.file(), loc.endPos() + 1, (u4)(loc.endPos() + 1 + toReplace.length())};
                                    e.replaceWith(fmt::format("Replace with `{}`", replacement), methodLoc, "{}",
                                                  replacement);
                                    addedAutocorrect = true;
                                }
                            }
                        }

                        if (!addedAutocorrect) {
                            lines.emplace_back(ErrorLine::from(alternative.symbol.data(ctx)->loc(),
                                                               "Did you mean: `{}`?", suggestedName));
                        }
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
            result.main.errors.emplace_back(e.build());
        }
        return result;
    }

    SymbolRef method = mayBeOverloaded.data(ctx)->isOverloaded()
                           ? guessOverload(ctx.withOwner(mayBeOverloaded), symbol, mayBeOverloaded, args.args,
                                           args.fullType, targs, args.block != nullptr)
                           : mayBeOverloaded;

    DispatchResult result;
    auto &component = result.main;
    component.receiver = args.selfType;
    component.method = method;

    const SymbolData data = method.data(ctx);
    unique_ptr<TypeConstraint> &maybeConstraint = result.main.constr;
    TypeConstraint *constr;
    if (args.block || data->isGenericMethod()) {
        maybeConstraint = make_unique<TypeConstraint>();
        constr = maybeConstraint.get();
    } else {
        constr = &TypeConstraint::EmptyFrozenConstraint;
    }

    if (data->isGenericMethod()) {
        constr->defineDomain(ctx, data->typeArguments());
    }
    bool hasKwargs = absl::c_any_of(data->arguments(), [](const auto &arg) { return arg.flags.isKeyword; });

    // p -> params, i.e., what was mentioned in the defintiion
    auto pit = data->arguments().begin();
    auto pend = data->arguments().end();

    ENFORCE(pit != pend, "Should at least have the block arg.");
    ENFORCE((pend - 1)->flags.isBlock, "Last arg should be the block arg: " + (pend - 1)->show(ctx));
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
            Types::approximate(ctx, arg->type, *constr)->derivesFrom(ctx, Symbols::Hash())) {
            break;
        }

        auto offset = ait - args.args.begin();
        if (auto e = matchArgType(ctx, *constr, args.locs.call, args.locs.receiver, symbol, method, *arg, spec,
                                  args.selfType, targs, args.locs.args[offset], args.args.size() == 1)) {
            result.main.errors.emplace_back(std::move(e));
        }

        if (!spec.flags.isRepeated) {
            ++pit;
        }
        ++ait;
    }

    if (pit != pend) {
        if (!(pit->flags.isKeyword || pit->flags.isDefault || pit->flags.isRepeated || pit->flags.isBlock)) {
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

                result.main.errors.emplace_back(e.build());
            }
        }
    }

    // keep this around so we know which keyword arguments have been supplied
    UnorderedSet<NameRef> consumed;
    if (hasKwargs && ait != aend) {
        auto &hashArg = *(aend - 1);
        auto hashArgType = Types::approximate(ctx, hashArg->type, *constr);

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
                        if (auto e = missingArg(ctx, args.locs.call, args.locs.receiver, method, spec)) {
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
                if (auto e = matchArgType(ctx, *constr, args.locs.call, args.locs.receiver, symbol, method, tpe, spec,
                                          args.selfType, targs, Loc::none())) {
                    result.main.errors.emplace_back(std::move(e));
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
                    result.main.errors.emplace_back(e.build());
                }
            }
        } else if (hashArgType->derivesFrom(ctx, Symbols::Hash())) {
            --aend;
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::UntypedSplat)) {
                e.setHeader("Passing a hash where the specific keys are unknown to a method taking keyword arguments");
                e.addErrorSection(ErrorSection("Got " + hashArgType->show(ctx) + " originating from:",
                                               hashArg->origins2Explanations(ctx)));
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
            if (auto e = missingArg(ctx, args.locs.call, args.locs.receiver, method, spec)) {
                result.main.errors.emplace_back(std::move(e));
            }
        }
    }

    if (ait != aend) {
        if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::MethodArgumentCountMismatch)) {
            if (!hasKwargs) {
                e.setHeader("Too many arguments provided for method `{}`. Expected: `{}`, got: `{}`", data->show(ctx),
                            prettyArity(ctx, method), args.args.size());
                e.addErrorLine(method.data(ctx)->loc(), "`{}` defined here", args.name.show(ctx));
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
                            data->show(ctx), prettyArity(ctx, method), posArgs);
                e.addErrorLine(method.data(ctx)->loc(), "`{}` defined here", args.name.show(ctx));

                // if there's an obvious first keyword argument that the user hasn't supplied, we can mention it
                // explicitly
                auto firstKeyword = absl::c_find_if(data->arguments(), [&consumed](const ArgInfo &arg) {
                    return arg.flags.isKeyword && arg.flags.isDefault && consumed.count(arg.name) == 0;
                });
                if (firstKeyword != data->arguments().end()) {
                    e.addErrorLine(args.locs.call,
                                   "`{}` has optional keyword arguments. Did you mean to provide a value for `{}`?",
                                   data->show(ctx), firstKeyword->argumentName(ctx));
                }
            }
            result.main.errors.emplace_back(e.build());
        }
    }

    if (args.block != nullptr) {
        ENFORCE(!data->arguments().empty(), "Every symbol must at least have a block arg: {}", data->show(ctx));
        const auto &bspec = data->arguments().back();
        ENFORCE(bspec.flags.isBlock, "The last symbol must be the block arg: {}", data->show(ctx));

        TypePtr blockType = Types::resultTypeAsSeenFrom(ctx, bspec.type, data->owner, symbol, targs);
        if (!blockType) {
            blockType = Types::untyped(ctx, method);
        }

        component.blockReturnType = Types::getProcReturnType(ctx, blockType);
        blockType = constr->isSolved() ? Types::instantiate(ctx, blockType, *constr)
                                       : Types::approximate(ctx, blockType, *constr);
        component.blockPreType = blockType;
        component.blockSpec = bspec.deepCopy();
    }

    TypePtr &resultType = result.returnType;

    if (method.data(ctx)->intrinsic != nullptr) {
        method.data(ctx)->intrinsic->apply(ctx, args, thisType, result);
        // the call could have overriden constraint
        if (result.main.constr || constr != &core::TypeConstraint::EmptyFrozenConstraint) {
            constr = result.main.constr.get();
        }
        if (constr == nullptr) {
            constr = &core::TypeConstraint::EmptyFrozenConstraint;
        }
    }

    if (resultType == nullptr) {
        if (args.args.size() == 1 && isSetter(ctx, method.data(ctx)->name)) {
            // assignments always return their right hand side
            resultType = args.args.front()->type;
        } else if (args.args.size() == 2 && method.data(ctx)->name == Names::squareBracketsEq()) {
            resultType = args.args[1]->type;
        } else {
            resultType =
                Types::resultTypeAsSeenFrom(ctx, method.data(ctx)->resultType, method.data(ctx)->owner, symbol, targs);
        }
    }
    if (args.block == nullptr) {
        // if block is there we do not attempt to solve the constaint. CFG adds an explicit solve
        // node that triggers constraint solving
        if (!constr->solve(ctx)) {
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::GenericMethodConstaintUnsolved)) {
                e.setHeader("Could not find valid instantiation of type parameters");
                result.main.errors.emplace_back(e.build());
            }
        }
        ENFORCE(!data->arguments().empty(), "Every method should at least have a block arg.");
        ENFORCE(data->arguments().back().flags.isBlock, "The last arg should be the block arg.");
        auto blockType = data->arguments().back().type;
        if (blockType && !core::Types::isSubType(ctx, core::Types::nilClass(), blockType)) {
            if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::BlockNotPassed)) {
                e.setHeader("`{}` requires a block parameter, but no block was passed", args.name.show(ctx));
                e.addErrorLine(method.data(ctx)->loc(), "defined here");
                result.main.errors.emplace_back(e.build());
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
        component.sendTp = resultType;
    }
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
    for (const auto &arg : data->arguments()) {
        if (arg.flags.isRepeated) {
            ENFORCE(args.empty(), "getCallArguments with positional and repeated args is not supported: {}",
                    data->toString(ctx));
            return Types::arrayOf(ctx, Types::resultTypeAsSeenFrom(ctx, arg.type, data->owner, klass, targs));
        }
        ENFORCE(!arg.flags.isKeyword, "getCallArguments does not support kwargs: {}", data->toString(ctx));
        if (arg.flags.isBlock) {
            continue;
        }
        args.emplace_back(Types::resultTypeAsSeenFrom(ctx, arg.type, data->owner, klass, targs));
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
            auto innerArgs = DispatchArgs{Names::initialize(), args.locs, args.args, wrapped, wrapped, args.block};
            auto original = wrapped->dispatchCall(ctx, innerArgs);
            original.returnType = wrapped;
            original.main.sendTp = wrapped;
            return original;
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
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        res.returnType = Types::untypedUntracked();
    }
} T_untyped;

class T_must : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.empty()) {
            return;
        }
        const auto loc = args.locs.call;
        if (!args.args[0]->type->isFullyDefined()) {
            if (auto e = ctx.state.beginError(loc, errors::Infer::BareTypeUsage)) {
                e.setHeader("T.must() applied to incomplete type `{}`", args.args[0]->type->show(ctx));
            }
            return;
        }
        auto ret = Types::approximateSubtract(ctx, args.args[0]->type, Types::nilClass());
        if (ret == args.args[0]->type) {
            if (auto e = ctx.state.beginError(loc, errors::Infer::InvalidCast)) {
                e.setHeader("T.must(): Expected a `T.nilable` type, got: `{}`", args.args[0]->type->show(ctx));
                const auto locWithoutTMust = Loc{loc.file(), loc.beginPos() + 7, loc.endPos() - 1};
                e.replaceWith("Remove `T.must`", loc, "{}", locWithoutTMust.source(ctx));
            }
        }
        res.returnType = move(ret);
    }
} T_must;

class T_any : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.empty()) {
            return;
        }

        TypePtr ret = Types::bottom();
        auto i = -1;
        for (auto &arg : args.args) {
            i++;
            auto ty = unwrapType(ctx, args.locs.args[i], arg->type);
            ret = Types::any(ctx, ret, ty);
        }

        res.returnType = make_type<MetaType>(move(ret));
    }
} T_any;

class T_all : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.empty()) {
            return;
        }

        TypePtr ret = Types::top();
        auto i = -1;
        for (auto &arg : args.args) {
            i++;
            auto ty = unwrapType(ctx, args.locs.args[i], arg->type);
            ret = Types::all(ctx, ret, ty);
        }

        res.returnType = make_type<MetaType>(move(ret));
    }
} T_all;

class T_revealType : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }

        if (auto e = ctx.state.beginError(args.locs.call, errors::Infer::RevealType)) {
            e.setHeader("Revealed type: `{}`", args.args[0]->type->showWithMoreInfo(ctx));
            e.addErrorSection(ErrorSection("From:", args.args[0]->origins2Explanations(ctx)));
        }
        res.returnType = args.args[0]->type;
    }
} T_revealType;

class T_nilable : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }

        res.returnType = make_type<MetaType>(
            Types::any(ctx, unwrapType(ctx, args.locs.args[0], args.args[0]->type), Types::nilClass()));
    }
} T_nilable;

class T_proc : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // NOTE: real validation done during infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} T_proc;

class Object_class : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        SymbolRef self = unwrapSymbol(thisType);
        auto singleton = self.data(ctx)->lookupSingletonClass(ctx);
        if (singleton.exists()) {
            res.returnType = make_type<ClassType>(singleton);
        } else {
            res.returnType = Types::classClass();
        }
    }
} Object_class;

class Class_new : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        SymbolRef self = unwrapSymbol(thisType);

        auto attachedClass = self.data(ctx)->attachedClass(ctx);
        if (!attachedClass.exists()) {
            if (self == Symbols::Class()) {
                // `Class.new(...)`, but it isn't a specific Class. We know
                // calling .new on a Class will yield some sort of Object
                attachedClass = Symbols::Object();
            } else {
                return;
            }
        }
        auto instanceTy = attachedClass.data(ctx)->externalType(ctx);
        DispatchArgs innerArgs{Names::initialize(), args.locs, args.args, instanceTy, instanceTy, args.block};
        auto dispatched = instanceTy->dispatchCall(ctx, innerArgs);

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
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        SymbolRef attachedClass;

        SymbolRef self = unwrapSymbol(thisType);
        attachedClass = self.data(ctx)->attachedClass(ctx);

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

        auto arity = attachedClass.data(ctx)->typeArity(ctx);
        if (attachedClass == Symbols::Hash()) {
            arity = 2;
        }
        if (attachedClass.data(ctx)->typeMembers().empty()) {
            return;
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

            auto memData = mem.data(ctx);

            auto *memType = cast_type<LambdaParam>(memData->resultType.get());
            ENFORCE(memType != nullptr);

            if (memData->isFixed()) {
                // Fixed args are implicitly applied, and won't consume type
                // arguments from the list that's supplied.
                targs.emplace_back(memType->upperBound);
            } else if (it != args.args.end()) {
                auto loc = args.locs.args[it - args.args.begin()];
                auto argType = unwrapType(ctx, loc, (*it)->type);
                bool validBounds = true;

                // Validate type parameter bounds.
                if (!Types::isSubType(ctx, argType, memType->upperBound)) {
                    validBounds = false;
                    if (auto e = ctx.state.beginError(loc, errors::Infer::GenericTypeParamBoundMismatch)) {
                        auto argStr = argType->show(ctx);
                        e.setHeader("`{}` cannot be used for type member `{}`", argStr, memData->showFullName(ctx));
                        e.addErrorLine(loc, "`{}` is not a subtype of `{}`", argStr, memType->upperBound->show(ctx));
                    }
                }

                if (!Types::isSubType(ctx, memType->lowerBound, argType)) {
                    validBounds = false;

                    if (auto e = ctx.state.beginError(loc, errors::Infer::GenericTypeParamBoundMismatch)) {
                        auto argStr = argType->show(ctx);
                        e.setHeader("`{}` cannot be used for type member `{}`", argStr, memData->showFullName(ctx));
                        e.addErrorLine(loc, "`{}` is not a subtype of `{}`", memType->lowerBound->show(ctx), argStr);
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
                targs.emplace_back(TupleType::build(ctx, tupleArgs));
            } else {
                targs.emplace_back(Types::untypedUntracked());
            }
        }

        res.returnType = make_type<MetaType>(make_type<AppliedType>(attachedClass, move(targs)));
    }
} T_Generic_squareBrackets;

class Magic_buildHash : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
} Magic_buildHash;

class Magic_buildArray : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.empty()) {
            res.returnType = Types::arrayOfUntyped();
            return;
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
            tuple = make_type<MetaType>(move(tuple));
        }
        res.returnType = move(tuple);
    }
} Magic_buildArray;

class Magic_expandSplat : public IntrinsicMethod {
    static TypePtr expandArray(Context ctx, const TypePtr &type, int expandTo) {
        if (auto *ot = cast_type<OrType>(type.get())) {
            return Types::any(ctx, expandArray(ctx, ot->left, expandTo), expandArray(ctx, ot->right, expandTo));
        }

        auto *tuple = cast_type<TupleType>(type.get());
        if (tuple == nullptr && core::Types::approximate(ctx, type, core::TypeConstraint::EmptyFrozenConstraint)
                                    ->derivesFrom(ctx, Symbols::Array())) {
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
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.size() != 3) {
            res.returnType = Types::arrayOfUntyped();
            return;
        }
        auto val = args.args.front()->type;
        auto *beforeLit = cast_type<LiteralType>(args.args[1]->type.get());
        auto *afterLit = cast_type<LiteralType>(args.args[2]->type.get());
        if (!(beforeLit->underlying()->derivesFrom(ctx, Symbols::Integer()) &&
              afterLit->underlying()->derivesFrom(ctx, Symbols::Integer()))) {
            res.returnType = Types::untypedUntracked();
            return;
        }
        int before = (int)beforeLit->value;
        int after = (int)afterLit->value;
        res.returnType = expandArray(ctx, val, before + after);
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
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
        if (!lit || !lit->derivesFrom(ctx, Symbols::Symbol())) {
            return;
        }
        NameRef fn(ctx.state, (u4)lit->value);
        if (args.args[2]->type->isUntyped()) {
            res.returnType = args.args[2]->type;
            return;
        }
        auto *tuple = cast_type<TupleType>(args.args[2]->type.get());
        if (tuple == nullptr) {
            if (auto e = ctx.state.beginError(args.locs.args[2], core::errors::Infer::UntypedSplat)) {
                e.setHeader("Splats are only supported where the size of the array is known statically");
            }
            return;
        }

        InlinedVector<TypeAndOrigins, 2> sendArgStore;
        InlinedVector<const TypeAndOrigins *, 2> sendArgs =
            Magic_callWithSplat::generateSendArgs(tuple, sendArgStore, args.locs.args[2]);
        InlinedVector<Loc, 2> sendArgLocs(tuple->elems.size(), args.locs.args[2]);
        CallLocs sendLocs{args.locs.call, args.locs.args[0], sendArgLocs};
        DispatchArgs innerArgs{fn, sendLocs, sendArgs, receiver->type, receiver->type, args.block};
        auto dispatched = receiver->type->dispatchCall(ctx, innerArgs);
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
        for (auto &err : dispatched.main.errors) {
            ctx.state._error(std::move(err));
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

    static void showLocationOfArgDefn(Context ctx, ErrorBuilder &e, TypePtr blockType,
                                      DispatchComponent &dispatchComp) {
        if (!dispatchComp.method.exists()) {
            return;
        }

        if (dispatchComp.method.data(ctx)->isClass()) {
            return;
        }

        const auto &methodArgs = dispatchComp.method.data(ctx)->arguments();
        ENFORCE(!methodArgs.empty());
        const auto &bspec = methodArgs.back();
        ENFORCE(bspec.flags.isBlock);
        e.addErrorSection(ErrorSection({
            ErrorLine::from(bspec.loc, "Method `{}` has specified `{}` as `{}`",
                            dispatchComp.method.data(ctx)->show(ctx), bspec.argumentName(ctx), blockType->show(ctx)),
        }));
    }

    static void simulateCall(Context ctx, const TypeAndOrigins *receiver, DispatchArgs innerArgs,
                             shared_ptr<SendAndBlockLink> link, TypePtr passedInBlockType, Loc callLoc, Loc blockLoc,
                             DispatchResult &res) {
        auto dispatched = receiver->type->dispatchCall(ctx, innerArgs);
        for (auto &err : dispatched.main.errors) {
            res.main.errors.emplace_back(std::move(err));
        }
        dispatched.main.errors.clear();
        // We use isSubTypeUnderConstraint here with a TypeConstraint, so that we discover the correct generic bounds
        // as we do the subtyping check.
        auto &constr = dispatched.main.constr;
        auto &blockPreType = dispatched.main.blockPreType;
        if (blockPreType && !Types::isSubTypeUnderConstraint(ctx, *constr, true, passedInBlockType, blockPreType)) {
            ClassType *passedInProcClass = cast_type<ClassType>(passedInBlockType.get());
            auto nonNilableBlockType = Types::dropSubtypesOf(ctx, blockPreType, Symbols::NilClass());
            if (passedInProcClass && passedInProcClass->symbol == Symbols::Proc() &&
                Types::isSubType(ctx, nonNilableBlockType, passedInBlockType)) {
                // If a block of unknown arity is passed in, but the function was declared with a known arity,
                // raise an error in strict mode.
                // This could occur, for example, when using Method#to_proc, since we type it as returning a `Proc`.
                if (auto e = ctx.state.beginError(blockLoc, errors::Infer::ProcArityUnknown)) {
                    e.setHeader("Cannot use a `{}` with unknown arity as a `{}`", "Proc", blockPreType->show(ctx));
                    if (!dispatched.secondary) {
                        Magic_callWithBlock::showLocationOfArgDefn(ctx, e, blockPreType, dispatched.main);
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
                e.setHeader("Expected `{}` but found `{}` for block argument", blockPreType->show(ctx),
                            passedInBlockType->show(ctx));
                if (!dispatched.secondary) {
                    Magic_callWithBlock::showLocationOfArgDefn(ctx, e, blockPreType, dispatched.main);
                }
            }
        }

        {
            auto it = &dispatched;
            while (it != nullptr) {
                if (it->main.method.exists() && !it->main.method.data(ctx)->isClass()) {
                    const auto &methodArgs = it->main.method.data(ctx)->arguments();
                    ENFORCE(!methodArgs.empty());
                    const auto &bspec = methodArgs.back();
                    ENFORCE(bspec.flags.isBlock);

                    auto bspecType = bspec.type;
                    if (bspecType) {
                        // This subtype check is here to discover the correct generic bounds.
                        Types::isSubTypeUnderConstraint(ctx, *constr, true, passedInBlockType, bspecType);
                    }
                }
                it = it->secondary.get();
            }
        }
        if (constr) {
            if (!constr->solve(ctx)) {
                if (auto e = ctx.state.beginError(callLoc, errors::Infer::GenericMethodConstaintUnsolved)) {
                    e.setHeader("Could not find valid instantiation of type parameters");
                }
                res.returnType = core::Types::untypedUntracked();
            }

            if (!constr->isEmpty() && constr->isSolved()) {
                dispatched.returnType = Types::instantiate(ctx, dispatched.returnType, *(constr));
            }
        }
        res.returnType = dispatched.returnType;
    }

public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
            if (auto e = ctx.state.beginError(args.locs.args[2], core::errors::Infer::GenericPassedAsBlock)) {
                e.setHeader("Passing generics as block arguments is not supported");
            }
            return;
        }

        auto *lit = cast_type<LiteralType>(args.args[1]->type.get());
        if (!lit || !lit->derivesFrom(ctx, Symbols::Symbol())) {
            return;
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
        auto link = make_shared<core::SendAndBlockLink>(fn, Magic_callWithBlock::argInfoByArity(blockArity));
        res.main.constr = make_unique<TypeConstraint>();

        DispatchArgs innerArgs{fn, sendLocs, sendArgs, receiver->type, receiver->type, link};

        Magic_callWithBlock::simulateCall(ctx, receiver, innerArgs, link, finalBlockType, args.locs.args[2],
                                          args.locs.call, res);
    }
} Magic_callWithBlock;

class Magic_callWithSplatAndBlock : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
        if (!lit || !lit->derivesFrom(ctx, Symbols::Symbol())) {
            return;
        }
        NameRef fn(ctx.state, (u4)lit->value);

        if (args.args[2]->type->isUntyped()) {
            res.returnType = args.args[2]->type;
            return;
        }
        auto *tuple = cast_type<TupleType>(args.args[2]->type.get());
        if (tuple == nullptr) {
            if (auto e = ctx.state.beginError(args.locs.args[2], core::errors::Infer::UntypedSplat)) {
                e.setHeader("Splats are only supported where the size of the array is known statically");
            }
            return;
        }

        if (core::cast_type<core::TypeVar>(args.args[3]->type.get())) {
            if (auto e = ctx.state.beginError(args.locs.args[3], core::errors::Infer::GenericPassedAsBlock)) {
                e.setHeader("Passing generics as block arguments is not supported");
            }
            return;
        }

        InlinedVector<TypeAndOrigins, 2> sendArgStore;
        InlinedVector<const TypeAndOrigins *, 2> sendArgs =
            Magic_callWithSplat::generateSendArgs(tuple, sendArgStore, args.locs.args[2]);
        InlinedVector<Loc, 2> sendArgLocs(tuple->elems.size(), args.locs.args[2]);
        CallLocs sendLocs{args.locs.call, args.locs.args[0], sendArgLocs};

        TypePtr finalBlockType =
            Magic_callWithBlock::typeToProc(ctx, args.args[3]->type, args.locs.call, args.locs.args[3]);
        std::optional<int> blockArity = Magic_callWithBlock::getArityForBlock(finalBlockType);
        auto link = make_shared<core::SendAndBlockLink>(fn, Magic_callWithBlock::argInfoByArity(blockArity));
        res.main.constr = make_unique<TypeConstraint>();

        DispatchArgs innerArgs{fn, sendLocs, sendArgs, receiver->type, receiver->type, link};

        Magic_callWithBlock::simulateCall(ctx, receiver, innerArgs, link, finalBlockType, args.locs.args[3],
                                          args.locs.call, res);
    }
} Magic_callWithSplatAndBlock;

class Magic_suggestUntypedConstantType : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        ENFORCE(args.args.size() == 1);
        auto ty = core::Types::widen(ctx, args.args.front()->type);
        auto loc = args.locs.args[0];
        if (auto e = ctx.state.beginError(loc, core::errors::Infer::UntypedConstantSuggestion)) {
            e.setHeader("Constants must have type annotations with `{}` when specifying `{}`", "T.let",
                        "# typed: strict");
            if (!ty->isUntyped() && loc.exists()) {
                e.replaceWith(fmt::format("Initialize as `{}`", ty->show(ctx)), loc, "T.let({}, {})", loc.source(ctx),
                              ty->show(ctx));
            }
        }
        res.returnType = move(ty);
    }
} Magic_suggestUntypedConstantType;

class DeclBuilderForProcs_void : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // NOTE: real validation done in infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} DeclBuilderForProcs_void;

class DeclBuilderForProcs_returns : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // NOTE: real validation done in infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} DeclBuilderForProcs_returns;

class DeclBuilderForProcs_params : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // NOTE: real validation done in infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} DeclBuilderForProcs_params;

class DeclBuilderForProcs_bind : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        // NOTE: real validation done in infer
        res.returnType = Types::declBuilderForProcsSingletonClass();
    }
} DeclBuilderForProcs_bind;

class Tuple_squareBrackets : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        auto *tuple = cast_type<TupleType>(thisType);
        ENFORCE(tuple);
        LiteralType *lit = nullptr;
        if (args.args.size() == 1) {
            lit = cast_type<LiteralType>(args.args.front()->type.get());
        }
        if (!lit || !lit->underlying()->derivesFrom(ctx, Symbols::Integer())) {
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
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        res.returnType = args.selfType;
    }
} Tuple_to_a;

class Tuple_concat : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
        res.returnType = TupleType::build(ctx, std::move(elems));
    }
} Tuple_concat;

class Shape_merge : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
                return;
            }
            ENFORCE(lt->literalKind == LiteralType::LiteralTypeKind::Integer, "depth arg must be an Integer literal");

            if (lt->value >= 0) {
                depth = lt->value;
            } else {
                // Negative values behave like no depth was given
                depth = INT64_MAX;
            }
        } else {
            // If our arity is off, then calls.cc will report an error due to mismatch with the RBI elsewhere, so we
            // don't need to do anything special here
            return;
        }

        res.returnType = Types::arrayOf(ctx, recursivelyFlattenArrays(ctx, element, depth));
    }
} Array_flatten;

class Array_compact : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
        res.returnType = Types::arrayOf(ctx, ret);
    }
} Array_compact;

class Kernel_proc : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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

class enumerable_to_h : public IntrinsicMethod {
public:
    // Forward Enumerable.to_h to RubyType.enumerable_to_h[self]
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
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
        for (auto &err : dispatched.main.errors) {
            res.main.errors.emplace_back(std::move(err));
        }
        dispatched.main.errors.clear();
        res.returnType = move(dispatched.returnType);
    }
} enumerable_to_h;

// statically determine things like `Integer === 3` to be true
class Module_tripleEq : public IntrinsicMethod {
public:
    void apply(Context ctx, DispatchArgs args, const Type *thisType, DispatchResult &res) const override {
        if (args.args.size() != 1) {
            return;
        }
        auto rhs = args.args[0]->type;
        if (rhs->isUntyped()) {
            res.returnType = rhs;
            return;
        }
        auto rc = Types::getRepresentedClass(ctx, thisType);
        // in most cases, thisType is T.class_of(rc). see test/testdata/class_not_class_of.rb for an edge case.
        if (rc == core::Symbols::noSymbol()) {
            res.returnType = Types::Boolean();
            return;
        }
        auto lhs = rc.data(ctx)->externalType(ctx);
        ENFORCE(!lhs->isUntyped(), "lhs of Module.=== must be typed");
        if (Types::isSubType(ctx, rhs, lhs)) {
            res.returnType = Types::trueClass();
            return;
        }
        if (Types::glb(ctx, rhs, lhs)->isBottom()) {
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

    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::buildHash(), &Magic_buildHash},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::buildArray(), &Magic_buildArray},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::expandSplat(), &Magic_expandSplat},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::callWithSplat(), &Magic_callWithSplat},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::callWithBlock(), &Magic_callWithBlock},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::callWithSplatAndBlock(), &Magic_callWithSplatAndBlock},
    {Symbols::Magic(), Intrinsic::Kind::Singleton, Names::suggestType(), &Magic_suggestUntypedConstantType},

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
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::to_a(), &Tuple_to_a},
    {Symbols::Tuple(), Intrinsic::Kind::Instance, Names::concat(), &Tuple_concat},

    {Symbols::Shape(), Intrinsic::Kind::Instance, Names::merge(), &Shape_merge},

    {Symbols::Array(), Intrinsic::Kind::Instance, Names::flatten(), &Array_flatten},
    {Symbols::Array(), Intrinsic::Kind::Instance, Names::compact(), &Array_compact},

    {Symbols::Kernel(), Intrinsic::Kind::Instance, Names::proc(), &Kernel_proc},
    {Symbols::Kernel(), Intrinsic::Kind::Instance, Names::lambda(), &Kernel_proc},

    {Symbols::Enumerable(), Intrinsic::Kind::Instance, Names::to_h(), &enumerable_to_h},

    {Symbols::Module(), Intrinsic::Kind::Instance, Names::tripleEq(), &Module_tripleEq},
};

} // namespace sorbet::core
