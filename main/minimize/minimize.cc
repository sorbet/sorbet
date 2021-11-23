#include "main/minimize/minimize.h"
#include "absl/strings/match.h"
#include <optional>

using namespace std;

namespace sorbet::realmain {

namespace {

const vector<core::SymbolRef> IGNORED_BY_SYMBOL = {
    // Kernel is very common and produces a lot of missing_methods churn, but is rarely necessary
    // for type-checking. (It also sometimes gets inferred even when Sorbet already statically knows
    // about Kernel, which can cause dangling references to deleted classes.)
    core::Symbols::Kernel(),
};

const vector<string_view> IGNORED_BY_FULL_NAME = {
    // These modules are Sorbet-internal and aren't necessary for static typechecking, so they
    // aren't necessary and they cause a fair bit of missing_methods churn
    "T::Private::Methods::MethodHooks",    "T::Private::Methods::SingletonMethodHooks",
    "T::Private::Abstract::Hooks",         "T::Private::MixesInClassMethods",
    "T::Private::Final::NoInherit",        "T::Private::Final::NoIncludeExtend",
    "T::Private::Sealed::NoIncludeExtend", "T::InterfaceWrapper::Helpers",
};

enum class OutputCategory {
    External = 1,
    Util,
    Opus,
    GraphQL,
    AutogenProto,
    Autogen,
    Flatfiles,
    Model,
    Mutator,
    Chalk,
};

OutputCategory outputCategoryFromClassName(string_view fullName) {
    if (absl::StrContains(fullName, "Flatfiles")) {
        return OutputCategory::Flatfiles;
    } else if (absl::StrContains(fullName, "::Mutator")) {
        return OutputCategory::Mutator;
    } else if (absl::StrContains(fullName, "::Model")) {
        return OutputCategory::Model;
    } else if (absl::StrContains(fullName, "::Model")) {
        if (absl::StartsWith(fullName, "Plaid")) {
            return OutputCategory::External;
        } else {
            return OutputCategory::Model;
        }
    } else if (absl::StrContains(fullName, "::Autogen::Proto")) {
        return OutputCategory::AutogenProto;
    } else if (absl::StrContains(fullName, "::Autogen")) {
        return OutputCategory::Autogen;
    } else if (absl::StrContains(fullName, "::GraphQL")) {
        return OutputCategory::GraphQL;
    } else if (absl::StartsWith(fullName, "Opus")) {
        return OutputCategory::Opus;
    } else if (absl::StartsWith(fullName, "Chalk")) {
        return OutputCategory::Chalk;
    } else if (absl::StartsWith(fullName, "PrisonGuard") || absl::StartsWith(fullName, "Critic") ||
               absl::StartsWith(fullName, "Primus")) {
        return OutputCategory::Util;
    } else {
        return OutputCategory::External;
    }
}

core::MethodRef closestOverridenMethod(const core::GlobalState &gs, core::SymbolRef enclosingClassSymbol,
                                       core::NameRef name) {
    auto enclosingClass = enclosingClassSymbol.data(gs);
    ENFORCE(enclosingClass->isClassOrModuleLinearizationComputed(), "Should have been linearized by resolver");

    for (const auto &mixin : enclosingClass->mixins()) {
        auto mixinMethod = mixin.data(gs)->findMember(gs, name);
        if (mixinMethod.exists()) {
            return mixinMethod.asMethodRef();
        }
    }

    auto superClass = enclosingClass->superClass();
    if (!superClass.exists()) {
        return core::Symbols::noMethod();
    }

    auto superMethod = superClass.data(gs)->findMember(gs, name);
    if (superMethod.exists()) {
        return superMethod.asMethodRef();
    } else {
        return closestOverridenMethod(gs, superClass, name);
    }
}

void writeClassDef(const core::GlobalState &rbiGS, options::PrinterConfig &outfile, core::SymbolRef rbiEntry,
                   bool &wroteClassDef) {
    while (rbiEntry.data(rbiGS)->isSingletonClass(rbiGS)) {
        rbiEntry = rbiEntry.data(rbiGS)->attachedClass(rbiGS);
    }
    auto defType = rbiEntry.data(rbiGS)->isClassOrModuleClass() ? "class" : "module";
    auto fullName = rbiEntry.show(rbiGS);
    auto cbase = outputCategoryFromClassName(fullName) == OutputCategory::External ? "::" : "";
    // TODO(jez) Does missing methods ignore super classes?
    outfile.fmt("{} {}{}\n", defType, cbase, fullName);
    wroteClassDef = true;
}

// TODO(jez) These names were chosen to match as closely as possible with names used in missing_methods.rbi.
// It's possible we want to change them.
//
// The code is very much not idiomatic Sorbet code. It's written this way to make it easier to
// reason about doing exactly what the Ruby implementation does, and we probably want to clean it up
// once we confirm that it works.

void serializeMethods(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                      options::PrinterConfig &outfile, core::SymbolRef sourceClass, core::SymbolRef rbiClass,
                      bool isSingleton, bool &wroteClassDef) {
    // TODO(jez) factor this into a helper
    auto sourceMembersByName = UnorderedMap<string, core::SymbolRef>{};
    if (sourceClass.exists()) {
        for (auto [sourceEntryName, sourceEntry] : sourceClass.data(sourceGS)->members()) {
            if (!sourceEntry.exists()) {
                ENFORCE(sourceClass == core::Symbols::root());
                continue;
            }
            // Using show not showRaw so that overloads are collapsed into one entry.
            sourceMembersByName[sourceEntryName.show(sourceGS)] = sourceEntry;
        }
    }

    for (auto rbiMembersIt : rbiClass.data(rbiGS)->membersStableOrderSlow(rbiGS)) {
        auto rbiEntryName = rbiMembersIt.first;
        auto rbiEntry = rbiMembersIt.second;
        if (!rbiEntry.data(rbiGS)->isMethod()) {
            continue;
        }

        if (sourceClass.exists()) {
            auto sourceEntryIt = sourceMembersByName.find(rbiEntryName.show(rbiGS));
            if (sourceEntryIt != sourceMembersByName.end()) {
                continue;
            }
        }

        if ((rbiEntryName == core::Names::new_() && rbiClass == core::Symbols::Class()) ||
            (rbiEntryName == core::Names::initialize() && rbiClass == core::Symbols::BasicObject())) {
            continue;
        }

        auto rbiEntryShortName = rbiEntryName.shortName(rbiGS);
        auto n = rbiEntryShortName.size();
        if ((n == 0) ||                                                     // (ask clang-format for newlines)
            (isdigit(rbiEntryShortName[0])) ||                              //
            (absl::StrContains(rbiEntryShortName.substr(1), '-')) ||        //
            (absl::StrContains(rbiEntryShortName.substr(1), '+')) ||        //
            (absl::StrContains(rbiEntryShortName.substr(0, n - 1), '?')) || //
            (absl::StrContains(rbiEntryShortName.substr(0, n - 1), '!')) || //
            (absl::StrContains(rbiEntryShortName.substr(0, n - 1), '=')) || //
            (absl::StrContains(rbiEntryShortName, '.')) ||                  //
            (absl::StrContains(rbiEntryShortName, ' ')) ||                  //
            (absl::StrContains(rbiEntryShortName, '/')) ||                  //
            (absl::StrContains(rbiEntryShortName, ',')) ||                  //
            (absl::StrContains(rbiEntryShortName, '[')) ||                  //
            (absl::StrContains(rbiEntryShortName, ']')) ||                  //
            (absl::StartsWith(rbiEntryShortName, "<")) ||                   //
            (absl::EndsWith(rbiEntryShortName, ">"))                        //
        ) {
            if ((rbiEntryShortName != "=="sv) &&  //
                (rbiEntryShortName != "==="sv) && //
                (rbiEntryShortName != "<=>"sv) && //
                (rbiEntryShortName != "!~"sv) &&  //
                (rbiEntryShortName != "=~"sv) &&  //
                (rbiEntryShortName != "[]"sv) &&  //
                (rbiEntryShortName != "[]="sv)    //
            ) {
                // TODO(jez) log invalid name?
                continue;
            }
        }

        auto rbiMethod = rbiEntry.asMethodRef();
        auto superMethod = closestOverridenMethod(rbiGS, rbiClass, rbiEntryName);
        if (superMethod.exists() && (rbiMethod.data(rbiGS)->isAbstract() == superMethod.data(rbiGS)->isAbstract())) {
            continue;
        }

        // TODO(jez) Can we forward the source_location comment from the RBI file?
        // Maybe we can ask the lexer for all the comments in a file, and store a list of Locs +
        // comments on a core::File to make that faster? And then potentially read that in namer
        // when entering the method.

        if (!wroteClassDef) {
            writeClassDef(rbiGS, outfile, rbiClass, wroteClassDef);
        }

        // TODO(jez) eventually might want to extend this to serialize any type information that was present
        // TODO(jez) You're already hard-coding only two levels of this (no singleton of singleton).
        // Get rid of the double pass, and just do instance methods and singleton class methods in one pass.
        outfile.fmt("  def {}{}(", isSingleton ? "self." : "", rbiEntryShortName);

        auto &rbiParameters = rbiMethod.data(rbiGS)->arguments();
        if (rbiParameters.size() == 3 && rbiParameters[1].name == core::Names::fwdKwargs()) {
            // The positional and block parameters get their names normalized to make overload
            // checking easier. The only reliable way to detect `...` syntax is by looking at the
            // `fwdKwargs` name at position 1.
            outfile.fmt("...");
        } else {
            auto first = true;
            for (auto &rbiParameter : rbiParameters) {
                if (first) {
                    first = false;
                } else if (!(rbiParameter.flags.isBlock && rbiParameter.isSyntheticBlockArgument())) {
                    outfile.fmt(", ");
                }

                ENFORCE(rbiParameter.name.exists());
                auto rbiParameterShortName = rbiParameter.argumentName(rbiGS);
                if (rbiParameter.flags.isKeyword && rbiParameter.flags.isRepeated) {
                    outfile.fmt("**{}", rbiParameterShortName);
                } else if (rbiParameter.flags.isRepeated) {
                    outfile.fmt("*{}", rbiParameterShortName);
                } else if (rbiParameter.flags.isKeyword) {
                    if (rbiParameter.flags.isDefault) {
                        outfile.fmt("{}: ::T.unsafe(nil)", rbiParameterShortName);
                    } else {
                        outfile.fmt("{}:", rbiParameterShortName);
                    }
                } else if (rbiParameter.flags.isBlock) {
                    if (!rbiParameter.isSyntheticBlockArgument()) {
                        outfile.fmt("&{}", rbiParameterShortName);
                    }
                } else if (rbiParameter.flags.isDefault) {
                    outfile.fmt("{}=::T.unsafe(nil)", rbiParameterShortName);
                } else {
                    outfile.fmt("{}", rbiParameterShortName);
                }
            }
        }

        outfile.fmt("); end\n");
    }
}

void serializeIncludes(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                       options::PrinterConfig &outfile, core::SymbolRef sourceClass, core::SymbolRef rbiClass,
                       bool isSingleton, bool &wroteClassDef) {
    auto sourceClassMixinsFullNames = vector<string>{};
    if (sourceClass.exists()) {
        for (auto sourceMixin : sourceClass.data(sourceGS)->mixins()) {
            sourceClassMixinsFullNames.emplace_back(sourceMixin.show(sourceGS));
        }
    }

    for (auto rbiMixin : rbiClass.data(rbiGS)->mixins()) {
        if (!rbiMixin.exists()) {
            continue;
        }

        auto rbiMixinFullName = rbiMixin.show(rbiGS);
        if (sourceClass.exists()) {
            // SymbolRef IDs are NOT COMPARABLE, because we have to resolve with separate GlobalStates
            // to know what is defined where. Have to compare with string names here.
            if (absl::c_any_of(sourceClassMixinsFullNames,
                               [&](const string &mixin) { return mixin == rbiMixinFullName; })) {
                continue;
            }
        }

        if (absl::c_any_of(IGNORED_BY_SYMBOL, [&](auto sym) { return sym == rbiMixin; })) {
            continue;
        }

        if (absl::c_any_of(IGNORED_BY_FULL_NAME, [&](auto fullName) { return fullName == rbiMixinFullName; })) {
            continue;
        }

        auto keyword = isSingleton ? "extend"sv : "include"sv;
        // Don't prefix pay-server includes with :: -- it will break rigorous packages.
        auto cbase = outputCategoryFromClassName(rbiMixinFullName) == OutputCategory::External ? "::" : "";

        if (!wroteClassDef) {
            writeClassDef(rbiGS, outfile, rbiClass, wroteClassDef);
        }

        outfile.fmt("  {} {}{}\n", keyword, cbase, rbiMixinFullName);
    }
}

void serializeClasses(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                      options::PrinterConfig &outfile, core::SymbolRef sourceClass, core::SymbolRef rbiClass) {
    ENFORCE(rbiClass.exists());

    auto sourceMembersByName = UnorderedMap<string, core::SymbolRef>{};
    if (sourceClass.exists()) {
        for (auto [sourceEntryName, sourceEntry] : sourceClass.data(sourceGS)->members()) {
            if (!sourceEntry.exists()) {
                ENFORCE(sourceClass == core::Symbols::root());
                continue;
            }
            sourceMembersByName[sourceEntryName.showRaw(sourceGS)] = sourceEntry;
        }
    }

    for (auto rbiMembersIt : rbiClass.data(rbiGS)->membersStableOrderSlow(rbiGS)) {
        auto rbiEntryName = rbiMembersIt.first;
        auto rbiEntry = rbiMembersIt.second;
        if (!rbiEntry.exists()) {
            ENFORCE(rbiClass == core::Symbols::root());
            continue;
        }

        // Special methods we use to store things on the symbol table. Not meant to be printed.
        if (rbiEntryName == core::Names::singleton() ||             //
            rbiEntryName == core::Names::attached() ||              //
            rbiEntryName == core::Names::mixedInClassMethods() ||   //
            rbiEntryName == core::Names::Constants::AttachedClass() //
        ) {
            continue;
        }

        if (!rbiEntry.data(rbiGS)->isClassOrModule()) {
            continue;
        }

        // TODO(jez) This is not actually used except to pass information to serializeMethods and
        // serializeIncludes, but those methods could easily recompute it from their arguemnts.
        auto myClassIsSingleton = rbiEntry.data(rbiGS)->isSingletonClass(rbiGS);

        // auto myClass = rbiEntry;
        if (rbiEntry == core::Symbols::T()) {
            // We specifically don't typecheck anything in T:: since it is hardcoded into sorbet
            continue;
        }

        auto sourceEntry = core::Symbols::noSymbol();
        if (sourceClass.exists()) {
            auto sourceEntryIt = sourceMembersByName.find(rbiEntryName.showRaw(rbiGS));
            if (sourceEntryIt != sourceMembersByName.end()) {
                sourceEntry = sourceEntryIt->second;
            }
        }

        if (sourceEntry.exists() && !sourceEntry.data(sourceGS)->isClassOrModule()) {
            // TODO(jez) It's not clear that these errors matter to log.
            fmt::print(stderr, "The source says {} is a {} but reflection says it is a {}\n", "TODO(jez)", "TODO(jez)",
                       "TODO(jez)");
            continue;
        }

        auto wroteClassDef = false;
        if (!sourceEntry.exists()) {
            writeClassDef(rbiGS, outfile, rbiEntry, wroteClassDef);
        }

        serializeIncludes(sourceGS, rbiGS, outfile, sourceEntry, rbiEntry, myClassIsSingleton, wroteClassDef);
        serializeMethods(sourceGS, rbiGS, outfile, sourceEntry, rbiEntry, myClassIsSingleton, wroteClassDef);
        // TODO(jez) the pay-server version doesn't handle static fields and type members

        if (wroteClassDef) {
            outfile.fmt("end\n\n");
        }

        serializeClasses(sourceGS, rbiGS, outfile, sourceEntry, rbiEntry);
    }
}

} // namespace

void Minimize::writeDiff(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                         options::PrinterConfig &outfile) {
    // TODO(jez) Use sed to post-process this to `typed: autogenerated` when you get to rolling this
    // out in pay-server. Also add a "do not edit" preamble.
    outfile.fmt("# typed: true\n\n");

    serializeClasses(sourceGS, rbiGS, outfile, core::Symbols::root(), core::Symbols::root());
}

} // namespace sorbet::realmain
