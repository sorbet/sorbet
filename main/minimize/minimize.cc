#include "main/minimize/minimize.h"
#include "absl/strings/match.h"
#include "common/EarlyReturnWithCode.h"
#include "core/ErrorQueue.h"
#include "main/pipeline/pipeline.h"
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
    "T::Private::Methods::MethodHooks", "T::Private::Methods::SingletonMethodHooks",
    "T::Private::Abstract::Hooks",      "T::Private::MixesInClassMethods",
    "T::Private::Final::NoInherit",     "T::Private::Final::NoIncludeExtend",
    "T::Private::Sealed::NoInherit",    "T::Private::Sealed::NoIncludeExtend",
    "T::InterfaceWrapper::Helpers",
};

// TODO(jez) This enum and function is a relic of the past. The only OutputCategory in use is
// `External`, and it's only for an attempt to know when to fully-qualify something in the presence
// of --stripe-packages mode.
//
// This could easily live as a post-process step in Stripe's codebase, and delete the code from here.
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
        if (absl::StartsWith(fullName, "Opus") || absl::StartsWith(fullName, "Chalk")) {
            return OutputCategory::Model;
        } else {
            return OutputCategory::External;
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

core::NameRef rbiNameToSourceName(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                                  core::NameRef rbiName) {
    switch (rbiName.kind()) {
        case core::NameKind::UTF8:
            return sourceGS.lookupNameUTF8(rbiName.dataUtf8(rbiGS)->utf8);
        case core::NameKind::CONSTANT:
            return sourceGS.lookupNameConstant(rbiNameToSourceName(sourceGS, rbiGS, rbiName.dataCnst(rbiGS)->original));
        case core::NameKind::UNIQUE:
            auto rbiUnique = rbiName.dataUnique(rbiGS);
            return sourceGS.lookupNameUnique(rbiUnique->uniqueNameKind,
                                             rbiNameToSourceName(sourceGS, rbiGS, rbiUnique->original), rbiUnique->num);
    }
}

void writeClassDef(const core::GlobalState &rbiGS, options::PrinterConfig &outfile, core::ClassOrModuleRef rbiEntry,
                   bool &wroteClassDef) {
    while (rbiEntry.data(rbiGS)->isSingletonClass(rbiGS)) {
        rbiEntry = rbiEntry.data(rbiGS)->attachedClass(rbiGS);
    }
    auto defType = rbiEntry.data(rbiGS)->isClass() ? "class" : "module";
    auto fullName = rbiEntry.show(rbiGS);
    auto cbase = outputCategoryFromClassName(fullName) == OutputCategory::External ? "::" : "";
    // TODO: The old Ruby-powered version did not emit superclasses. Eventually, we might want to
    // expand this to serialize the superclass if it was not seen in sourceGS
    outfile.fmt("{} {}{}\n", defType, cbase, fullName);
    wroteClassDef = true;
}

void serializeMethods(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                      options::PrinterConfig &outfile, core::ClassOrModuleRef sourceClass,
                      core::ClassOrModuleRef rbiClass, bool &wroteClassDef) {
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
        auto maybeRbiEntry = rbiMembersIt.second;
        if (!maybeRbiEntry.isMethod()) {
            continue;
        }
        auto rbiEntry = maybeRbiEntry.asMethodRef();

        if (sourceClass.exists()) {
            if (sourceMembersByName.contains(rbiEntryName.show(rbiGS))) {
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
                (rbiEntryShortName != "<"sv) &&   //
                (rbiEntryShortName != "<="sv) &&  //
                (rbiEntryShortName != ">"sv) &&   //
                (rbiEntryShortName != ">="sv) &&  //
                (rbiEntryShortName != "!~"sv) &&  //
                (rbiEntryShortName != "=~"sv) &&  //
                (rbiEntryShortName != "<<"sv) &&  //
                (rbiEntryShortName != "[]"sv) &&  //
                (rbiEntryShortName != "[]="sv)    //
            ) {
                // The Ruby-powered version would have printed a warning here, but we're going to
                // skip that because there's not an obvious place to emit the log, and we're going
                // to swallow these errors downstream anyways.
                continue;
            }
        }

        if (sourceClass.exists()) {
            // We already `continue`'d earlier for this case. If this ENFORCE were not true, findMemberTransitive
            // will not actually find a super method, just the same method in sourceGS.
            ENFORCE(sourceMembersByName.find(rbiEntryName.show(rbiGS)) == sourceMembersByName.end());

            // Have to find the superMethod in sourceGS, because otherwise the `isAbstract` bit won't be set
            auto sourceSuperMethod = sourceClass.data(sourceGS)->findMethodTransitive(
                sourceGS, rbiNameToSourceName(sourceGS, rbiGS, rbiEntryName));
            if (sourceSuperMethod.exists() && !sourceSuperMethod.data(sourceGS)->flags.isAbstract) {
                // Sorbet will fall back to dispatching to the parent method, which might have a sig.
                // But if the parent is abstract, and we don't serialize a method, Sorbet will
                // complain about the method not being implemented when it was, just not visibly.
                continue;
            }

            if (sourceClass.data(sourceGS)->flags.isAbstract && rbiEntryName == core::Names::initialize()) {
                // `abstract!` will define `initialize` in the class to raise unconditionally
                // https://github.com/sorbet/sorbet/blob/026c60bf719d/gems/sorbet-runtime/lib/types/private/abstract/declare.rb#L37-L42
                // Which is not useful to include in the minimized output.
                continue;
            }
        }

        // TODO: The old Ruby-powered version used runtime reflection to record a comment like
        //
        //     # definition: lib/foo:123
        //
        // We could consider looking for these comments in the opts.minimizeRBI input file, and
        // forwarding them to our output. For the time being, I've decided not to do this because I
        // suspect the naive solution would be too slow.
        //
        // We might be able to do something like associate a `core::Loc` with each definition
        // recording the source range corresponding to the doc comment for that definition, which
        // would make our work here easy, as well as in LSP for hover requests.

        if (!wroteClassDef) {
            writeClassDef(rbiGS, outfile, rbiClass, wroteClassDef);
        }

        // TODO: The old Ruby-powered version never needed to process RBI files that had sigs or
        // other type information, so this Sorbet version also ignores it. In the future, we should
        // probably update this pass to also serialize any new type information.

        auto isSingleton = rbiClass.data(rbiGS)->isSingletonClass(rbiGS);
        auto isPrivate = rbiEntry.data(rbiGS)->flags.isPrivate;
        outfile.fmt("  {}def {}{}(", isPrivate ? "private " : "", isSingleton ? "self." : "", rbiEntryShortName);

        auto &rbiParameters = rbiEntry.data(rbiGS)->arguments;
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
                        outfile.fmt("{}: T.unsafe(nil)", rbiParameterShortName);
                    } else {
                        outfile.fmt("{}:", rbiParameterShortName);
                    }
                } else if (rbiParameter.flags.isBlock) {
                    if (!rbiParameter.isSyntheticBlockArgument()) {
                        outfile.fmt("&{}", rbiParameterShortName);
                    }
                } else if (rbiParameter.flags.isDefault) {
                    outfile.fmt("{}=T.unsafe(nil)", rbiParameterShortName);
                } else {
                    outfile.fmt("{}", rbiParameterShortName);
                }
            }
        }

        outfile.fmt("); end\n");
    }
}

void serializeIncludes(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                       options::PrinterConfig &outfile, core::ClassOrModuleRef sourceClass,
                       core::ClassOrModuleRef rbiClass, bool &wroteClassDef) {
    ENFORCE(rbiClass.exists());
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

        auto isSingleton = rbiClass.data(rbiGS)->isSingletonClass(rbiGS);
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
                      options::PrinterConfig &outfile, core::ClassOrModuleRef sourceClass,
                      core::ClassOrModuleRef rbiClass) {
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
        auto maybeRbiClass = rbiMembersIt.second;
        if (!maybeRbiClass.exists()) {
            ENFORCE(rbiClass == core::Symbols::root());
            continue;
        }

        if (!maybeRbiClass.isClassOrModule()) {
            continue;
        }

        auto rbiEntry = maybeRbiClass.asClassOrModuleRef();
        if (rbiEntry.data(rbiGS)->isSingletonClass(rbiGS)) {
            // We're going to have serializeIncludes and serializeMethods serialize both the
            // instance and singleton methods at the same time.
            continue;
        }

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

        if (sourceEntry.exists() && !sourceEntry.isClassOrModule()) {
            // The Ruby-powered version would have printed a warning here, but we're going to
            // skip that because there's not an obvious place to emit the log, and we're going
            // to swallow these errors downstream anyways.
            continue;
        }

        auto sourceEntryClass = sourceEntry.asClassOrModuleRef();
        auto wroteClassDef = false;
        if (!sourceEntry.exists()) {
            writeClassDef(rbiGS, outfile, rbiEntry, wroteClassDef);
        }

        auto sourceEntrySingleton = sourceEntry.exists()
                                        ? sourceEntryClass.data(sourceGS)->lookupSingletonClass(sourceGS)
                                        : core::Symbols::noClassOrModule();
        auto rbiEntrySingleton = rbiEntry.data(rbiGS)->lookupSingletonClass(rbiGS);

        serializeIncludes(sourceGS, rbiGS, outfile, sourceEntryClass, rbiEntry, wroteClassDef);
        if (rbiEntrySingleton.exists()) {
            serializeIncludes(sourceGS, rbiGS, outfile, sourceEntrySingleton, rbiEntrySingleton, wroteClassDef);
        }
        serializeMethods(sourceGS, rbiGS, outfile, sourceEntryClass, rbiEntry, wroteClassDef);
        if (rbiEntrySingleton.exists()) {
            serializeMethods(sourceGS, rbiGS, outfile, sourceEntrySingleton, rbiEntrySingleton, wroteClassDef);
        }

        // TODO: The old Ruby-powered version never attempted to handle static fields nor type
        // members, because those never showed up in any RBI files handed to it.
        // It would be useful to add support for those here eventually.

        if (wroteClassDef) {
            outfile.fmt("end\n\n");
        }

        serializeClasses(sourceGS, rbiGS, outfile, sourceEntryClass, rbiEntry);
    }
}

} // namespace

void Minimize::indexAndResolveForMinimize(unique_ptr<core::GlobalState> &sourceGS, unique_ptr<core::GlobalState> &rbiGS,
                                          options::Options &opts, WorkerPool &workers, std::string minimizeRBI) {
    Timer timeit(sourceGS->tracer(), "Minimize::indexAndResolveForMinimize");

    ENFORCE(!sourceGS->findFileByPath(minimizeRBI).exists(),
            "--minimize-to-rbi will yield empty file because {} was already processed by the main pipeline",
            minimizeRBI);

    auto rbiInputFiles = pipeline::reserveFiles(rbiGS, {minimizeRBI});

    // I'm ignoring everything relating to caching here, because missing methods is likely
    // to run on a new _unknown.rbi file every time and I didn't want to think about it.
    // If this phase gets slow, we can consider whether caching would speed things up.
    auto rbiIndexed = pipeline::index(*rbiGS, absl::Span<core::FileRef>(rbiInputFiles), opts, workers, nullptr);
    if (rbiGS->hadCriticalError()) {
        rbiGS->errorQueue->flushAllErrors(*rbiGS);
    }

    pipeline::setPackagerOptions(*rbiGS, opts);
    pipeline::package(*rbiGS, absl::Span<ast::ParsedFile>(rbiIndexed), opts, workers);
    // Only need to compute FoundDefHashes when running to compute a FileHash
    auto foundHashes = nullptr;
    auto canceled = pipeline::name(*rbiGS, absl::Span<ast::ParsedFile>(rbiIndexed), opts, workers, foundHashes);
    ENFORCE(!canceled, "Can only cancel in LSP mode");

    rbiIndexed = move(pipeline::resolve(rbiGS, move(rbiIndexed), opts, workers).result());
    if (rbiGS->hadCriticalError()) {
        rbiGS->errorQueue->flushAllErrors(*rbiGS);
    }

    rbiGS->errorQueue->flushAllErrors(*rbiGS);

    // rbiIndexed goes out of scope here, and destructors run
    // If this becomes too slow, we can consider using intentionallyLeakMemory
}

void Minimize::writeDiff(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                         options::PrinterConfig &outfile) {
    Timer timeit(sourceGS.tracer(), "Minimize::writeDiff");

    outfile.fmt("# typed: true\n\n");

    serializeClasses(sourceGS, rbiGS, outfile, core::Symbols::root(), core::Symbols::root());
}

} // namespace sorbet::realmain
