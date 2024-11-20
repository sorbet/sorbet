#include "core/TypeErrorDiagnostics.h"
#include "absl/algorithm/container.h"
#include "absl/strings/str_join.h"
#include "core/errors/infer.h"

using namespace std;

namespace sorbet::core {

namespace {

[[nodiscard]] bool checkForAttachedClassHint(const GlobalState &gs, ErrorBuilder &e, const SelfTypeParam expected,
                                             const ClassType got) {
    if (expected.definition.name(gs) != Names::Constants::AttachedClass()) {
        return false;
    }

    auto attachedClass = got.symbol.data(gs)->lookupSingletonClass(gs);
    if (!attachedClass.exists()) {
        return false;
    }

    if (attachedClass != expected.definition.owner(gs).asClassOrModuleRef()) {
        return false;
    }

    auto gotStr = got.show(gs);
    auto expectedStr = expected.show(gs);
    e.addErrorNote(
        "`{}` is incompatible with `{}` because when this method is called on a subclass `{}` will represent a more "
        "specific subclass, meaning `{}` will not be specific enough. See https://sorbet.org/docs/attached-class for "
        "more.",
        gotStr, expectedStr, expectedStr, gotStr);
    return true;
}
} // namespace

void TypeErrorDiagnostics::explainTypeMismatch(const GlobalState &gs, ErrorBuilder &e,
                                               const ErrorSection::Collector &collector, const TypePtr &expected,
                                               const TypePtr &got) {
    e.addErrorSections(std::move(collector));
    // TODO: the rest of this function should eventually be moved to isSubTypeUnderConstraint,
    // as calls to collector.addErrorDetails
    auto expectedSelfTypeParam = isa_type<SelfTypeParam>(expected);
    auto gotClassType = isa_type<ClassType>(got);
    if (expectedSelfTypeParam && gotClassType) {
        if (checkForAttachedClassHint(gs, e, cast_type_nonnull<SelfTypeParam>(expected),
                                      cast_type_nonnull<ClassType>(got))) {
            return;
        }
    }

    if (isa_type<MetaType>(got) && !isa_type<MetaType>(expected)) {
        e.addErrorNote(
            "It looks like you're using Sorbet type syntax in a runtime value position.\n"
            "    If you really mean to use types as values, use `{}` to hide the type syntax from the type checker.\n"
            "    Otherwise, you're likely using the type system in a way it wasn't meant to be used.",
            "T::Utils.coerce");
        return;
    }

    // TODO(jez) Add more cases
}

void TypeErrorDiagnostics::maybeAutocorrect(const GlobalState &gs, ErrorBuilder &e, Loc loc,
                                            const TypeConstraint &constrOrig, const TypePtr &expectedType,
                                            const TypePtr &actualType) {
    if (!loc.exists()) {
        return;
    }

    if (gs.suggestUnsafe.has_value()) {
        e.replaceWith(fmt::format("Wrap in `{}`", *gs.suggestUnsafe), loc, "{}({})", *gs.suggestUnsafe,
                      loc.source(gs).value());
    } else {
        // Duplicate the current constraint, so that we don't record any additional constraints
        // simply from trying to generate autocorrects, and then solve it so that each additional
        // heuristic here doesn't interfere with later heuristics.
        auto constr = constrOrig.deepCopy();
        if (!constr->solve(gs)) {
            // Constraint already doesn't solve. This will be an error later in infer.
            // For now, let's just give up on our type-driven autocorrects.
            return;
        }

        auto withoutNil = Types::dropNil(gs, actualType);
        if (!withoutNil.isBottom() &&
            Types::isSubTypeUnderConstraint(gs, *constr, withoutNil, expectedType, UntypedMode::AlwaysCompatible,
                                            ErrorSection::Collector::NO_OP)) {
            e.replaceWith("Wrap in `T.must`", loc, "T.must({})", loc.source(gs).value());
        } else if (isa_type<MetaType>(actualType) && !isa_type<MetaType>(expectedType) &&
                   core::Types::isSubTypeUnderConstraint(
                       gs, *constr, core::Symbols::T_Types_Base().data(gs)->externalType(), expectedType,
                       UntypedMode::AlwaysCompatible, ErrorSection::Collector::NO_OP)) {
            e.replaceWith("Wrap in `T::Utils.coerce`", loc, "T::Utils.coerce({})", loc.source(gs).value());
        }
    }
}

void TypeErrorDiagnostics::insertTypeArguments(const GlobalState &gs, ErrorBuilder &e, ClassOrModuleRef klass,
                                               core::Loc replaceLoc) {
    // if we're looking at `Array`, we want the autocorrect to include `T::`, but we don't need to
    // if we're already looking at `T::Array` instead.
    klass = klass.maybeUnwrapBuiltinGenericForwarder();
    auto typePrefixSym = klass.forwarderForBuiltinGeneric();
    if (!typePrefixSym.exists()) {
        typePrefixSym = klass;
    }

    auto loc = replaceLoc;
    if (loc.exists()) {
        if (klass == core::Symbols::Hash() || klass == core::Symbols::T_Hash()) {
            // Hash is special because it has arity 3 but you're only supposed to write the first 2
            e.replaceWith("Add type arguments", loc, "{}[T.untyped, T.untyped]", typePrefixSym.show(gs));
        } else {
            auto numTypeArgs = klass.data(gs)->typeArity(gs);
            auto arg = (klass == Symbols::Class() || klass == Symbols::T_Class()) ? "T.anything" : "T.untyped";
            vector<string> untypeds;
            for (int i = 0; i < numTypeArgs; i++) {
                untypeds.emplace_back(arg);
            }
            e.replaceWith("Add type arguments", loc, "{}[{}]", typePrefixSym.show(gs), absl::StrJoin(untypeds, ", "));
        }
    }
}

void TypeErrorDiagnostics::explainUntyped(const GlobalState &gs, ErrorBuilder &e, ErrorClass what,
                                          const TypeAndOrigins &untyped, Loc originForUninitialized) {
    e.addErrorSection(untyped.explainGot(gs, originForUninitialized));

    if constexpr (sorbet::track_untyped_blame_mode || sorbet::debug_mode) {
        auto blameSymbol = untyped.type.untypedBlame();
        if (blameSymbol.exists()) {
            auto loc = blameSymbol.loc(gs);
            if (loc.exists()) {
                e.addErrorLine(loc, "Blames to `{}`, defined here", blameSymbol.show(gs));
            } else {
                e.addErrorNote("Blames to `{}`", blameSymbol.show(gs));
            }
        } else {
            e.addErrorNote("Blames to `{}`", "<none>");
        }
    }

    if (what == core::errors::Infer::UntypedValue) {
        e.addErrorNote("Support for `{}` is minimal. Consider using `{}` instead.", "typed: strong", "typed: strict");
    }
}

namespace {
optional<core::AutocorrectSuggestion::Edit> autocorrectEditForDSLMethod(const GlobalState &gs, Loc insertLoc,
                                                                        string_view prefix, ClassOrModuleRef dslOwner,
                                                                        string_view dsl, bool needsDslOwner) {
    if (needsDslOwner) {
        if (dsl == "") {
            return core::AutocorrectSuggestion::Edit{
                insertLoc,
                fmt::format("{}extend {}\n", prefix, dslOwner.show(gs)),
            };
        } else {
            return core::AutocorrectSuggestion::Edit{
                insertLoc,
                fmt::format("{}extend {}\n{}{}\n", prefix, dslOwner.show(gs), prefix, dsl),
            };
        }
    } else if (dsl != "") {
        return core::AutocorrectSuggestion::Edit{
            insertLoc,
            fmt::format("{}{}\n", prefix, dsl),
        };
    } else {
        return nullopt;
    }
}
} // namespace

// dslOwner can be noClassOrModule() to simply unconditionally insert the `dsl` string
// dsl can be `""` to simply insert `extend {dslOwner}` if dslOwner is not already an ancestor
optional<core::AutocorrectSuggestion::Edit>
TypeErrorDiagnostics::editForDSLMethod(const GlobalState &gs, FileRef fileToEdit, Loc defaultInsertLoc,
                                       ClassOrModuleRef inWhatRef, ClassOrModuleRef dslOwner, string_view dsl) {
    auto inWhat = inWhatRef.data(gs);
    auto inWhatSingleton = inWhat->lookupSingletonClass(gs);

    auto needsDslOwner = false;
    if (dslOwner.exists()) {
        needsDslOwner = !inWhatSingleton.data(gs)->derivesFrom(gs, dslOwner);
    }

    auto inCurrentFile = [&](const auto &loc) { return loc.file() == fileToEdit; };
    auto classLocs = inWhat->locs();
    auto classLoc = absl::c_find_if(classLocs, inCurrentFile);

    if (classLoc == classLocs.end()) {
        if ((inWhatRef == Symbols::root() || inWhatRef == Symbols::Object()) && defaultInsertLoc.exists()) {
            // We don't put any locs on <root> for performance (would have one loc for each file in the codebase)
            // If they're writing methods at the top level, it's probably a small script.
            // Just put the `extend` immediately above the sig.

            auto [sigStart, _sigEnd] = defaultInsertLoc.position(gs);
            auto thisLineStart = core::Loc::Detail{sigStart.line, 1};
            auto thisLineLoc = core::Loc::fromDetails(gs, defaultInsertLoc.file(), thisLineStart, thisLineStart);
            ENFORCE(thisLineLoc.has_value());
            auto [_, thisLinePadding] = thisLineLoc.value().findStartOfLine(gs);

            string prefix(thisLinePadding, ' ');
            return autocorrectEditForDSLMethod(gs, thisLineLoc.value(), prefix, dslOwner, dsl, needsDslOwner);
        } else {
            return nullopt;
        }
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
    return autocorrectEditForDSLMethod(gs, nextLineLoc.value(), prefix, dslOwner, dsl, needsDslOwner);
}

void TypeErrorDiagnostics::maybeInsertDSLMethod(const GlobalState &gs, ErrorBuilder &e, FileRef fileToEdit,
                                                Loc defaultInsertLoc, ClassOrModuleRef inWhatRef,
                                                ClassOrModuleRef dslOwner, string_view dsl) {
    auto edit = editForDSLMethod(gs, fileToEdit, defaultInsertLoc, inWhatRef, dslOwner, dsl);
    if (!edit.has_value()) {
        return;
    }

    auto label = dsl == "" ? fmt::format("Add `extend {}`", dslOwner.show(gs)) : fmt::format("Insert `{}`", dsl);

    e.addAutocorrect(core::AutocorrectSuggestion{move(label), {move(edit.value())}});
}

} // namespace sorbet::core
