#include "main/minimize/minimize.h"
#include "absl/strings/match.h"
#include <optional>

using namespace std;

namespace sorbet::realmain {

namespace {

// TODO(jez) Might make sense to just factor this into Symbols.h
enum class SymbolKind {
    UNKNOWN_TYPE = 0,
    CLASS_OR_MODULE = 1,
    STATIC_FIELD = 2,
    FIELD = 3,
    METHOD = 4,
    // ARGUMENT = 5,
    TYPE_MEMBER = 6,
    TYPE_ARGUMENT = 7,
};

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

void writeClassDef(const core::GlobalState &rbiGS, options::PrinterConfig &outfile, core::SymbolRef rbiEntry) {
    auto defType = rbiEntry.data(rbiGS)->isClassOrModuleClass() ? "class" : "module";
    // TODO(jez) Does missing methods ignore super classes?
    outfile.fmt("{} ::{}\n", defType, rbiEntry.data(rbiGS)->name.shortName(rbiGS));
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
    for (auto rbiMembersIt : rbiClass.data(rbiGS)->membersStableOrderSlow(rbiGS)) {
        auto rbiEntryName = rbiMembersIt.first;
        auto rbiEntry = rbiMembersIt.second;
        if (!rbiEntry.data(rbiGS)->isMethod()) {
            continue;
        }

        if (sourceClass.exists()) {
            // Mixes names entered across global states, which is why it's important equal names have
            // equal IDs across GlobalStates.
            auto sourceEntry = sourceClass.data(sourceGS)->findMember(sourceGS, rbiEntryName);
            if (sourceEntry.exists()) {
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
            writeClassDef(rbiGS, outfile, rbiClass);
            wroteClassDef = true;
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
                } else {
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

void serializeClasses(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                      options::PrinterConfig &outfile, core::SymbolRef sourceClass, core::SymbolRef rbiClass) {
    ENFORCE(rbiClass.exists());
    for (auto rbiMembersIt : rbiClass.data(rbiGS)->membersStableOrderSlow(rbiGS)) {
        auto rbiEntryName = rbiMembersIt.first;
        auto rbiEntry = rbiMembersIt.second;
        if (!rbiEntry.exists()) {
            // Symbols::noSymbol() is a valid symbol owned by root that we have to skip
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
        // if (myClassIsSingleton && absl::StartsWith(rbiEntryName.shortName(rbiGS), "OnlyInSecond")) {
        //     stopInDebugger();
        // }

        // auto myClass = rbiEntry;
        if (rbiEntry == core::Symbols::T()) {
            // We specifically don't typecheck anything in T:: since it is hardcoded into sorbet
            continue;
        }

        auto sourceEntry = core::Symbols::noSymbol();
        if (sourceClass.exists()) {
            sourceEntry = sourceClass.data(sourceGS)->findMember(sourceGS, rbiEntryName);
        }

        if (sourceEntry.exists() && !sourceEntry.data(sourceGS)->isClassOrModule()) {
            // TODO(jez) It's not clear that these errors matter to log.
            fmt::print(stderr, "The source says {} is a {} but reflection says it is a {}\n", "TODO(jez)", "TODO(jez)",
                       "TODO(jez)");
            continue;
        }

        auto wroteClassDef = false;
        if (!sourceEntry.exists()) {
            writeClassDef(rbiGS, outfile, rbiEntry);
            wroteClassDef = true;
        }

        serializeMethods(sourceGS, rbiGS, outfile, sourceEntry, rbiEntry, myClassIsSingleton, wroteClassDef);
        // TODO(jez) serialize_includes
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
