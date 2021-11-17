#include "main/minimize/minimize.h"
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

SymbolKind getSymbolKind(const core::GlobalState &gs, core::SymbolRef sym) {
    auto data = sym.data(gs);
    if (data->isClassOrModule()) {
        return SymbolKind::CLASS_OR_MODULE;
    } else if (data->isStaticField()) {
        return SymbolKind::STATIC_FIELD;
    } else if (data->isField()) {
        return SymbolKind::FIELD;
    } else if (data->isMethod()) {
        return SymbolKind::METHOD;
    } else if (data->isTypeMember()) {
        return SymbolKind::TYPE_MEMBER;
    } else if (data->isTypeArgument()) {
        return SymbolKind::TYPE_ARGUMENT;
    } else {
        return SymbolKind::UNKNOWN_TYPE;
    }
}

// TODO(jez) These names were chosen to match as closely as possible with names used in missing_methods.rbi.
// It's possible we want to change them.
//
// The code is very much not idiomatic Sorbet code. It's written this way to make it easier to
// reason about doing exactly what the Ruby implementation does, and we probably want to clean it up
// once we confirm that it works.

void serializeClasses(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                      options::PrinterConfig &outfile, const UnorderedMap<core::NameRef, core::SymbolRef> &source,
                      const UnorderedMap<core::NameRef, core::SymbolRef> &rbi, core::SymbolRef sourceClass,
                      core::SymbolRef rbiClass) {
    auto sourceByName = source;

    for (auto [rbiEntryName, rbiEntry] : rbi) {
        if (!rbiEntry.exists()) {
            continue;
        }

        if (!rbiEntry.data(rbiGS)->isClassOrModule()) {
            continue;
        }

        auto name = rbiEntryName;
        auto myClassIsSingleton = false;
        if (rbiEntry.data(rbiGS)->isSingletonClass(rbiGS)) {
            auto rbiEntryAttached = rbiEntry.data(rbiGS)->attachedClass(rbiGS);
            name = rbiEntryAttached.data(rbiGS)->name;
            myClassIsSingleton = true;
        }

        // auto myClass = rbiEntry;
        if (rbiEntry == core::Symbols::T()) {
            // We specifically don't typecheck anything in T:: since it is hardcoded into sorbet
            continue;
        }

        auto sourceEntryIt = sourceByName.find(rbiEntryName);

        // TODO(jez) This is all overkill: it's just trying to check whether the thing is a class or
        // module symbol in sourceGS, and to skip to the next entry otherwise. There are better ways
        // to do this.
        auto sourceType = SymbolKind::UNKNOWN_TYPE;
        if (sourceEntryIt == sourceByName.end()) {
            auto sourceIt = sourceByName.find(name);
            if (sourceIt != sourceByName.end()) {
                sourceType = getSymbolKind(sourceGS, sourceIt->second);
            }
        } else {
            sourceType = getSymbolKind(sourceGS, sourceEntryIt->second);
        }
        if (sourceType != SymbolKind::UNKNOWN_TYPE && sourceType != SymbolKind::CLASS_OR_MODULE) {
            // TODO(jez) It's not clear that these errors matter to log.
            fmt::print(stderr, "The source says {} is a {} but reflection says it is a {}\n", "TODO(jez)", "TODO(jez)",
                       "TODO(jez)");
            continue;
        }

        auto sourceChildren = UnorderedMap<core::NameRef, core::SymbolRef>{};
        // TODO(jez) sourceMixins
        if (sourceEntryIt != sourceByName.end()) {
            sourceChildren = sourceEntryIt->second.data(sourceGS)->members();
            // TODO(jez) sourceMixins
        }
        auto rbiChildren = rbiEntry.data(rbiGS)->members();
        // TODO(jez) rbiMixins

        // TODO(jez) serialize_methods
        // TODO(jez) serialize_includes

        // TODO(jez) this is not actually the condition it uses to figure out whether to print or not
        if (sourceEntryIt == sourceByName.end()) {
            auto defType = rbiEntry.data(rbiGS)->isClassOrModuleClass() ? "class" : "module";
            // TODO(jez) Does missing methods ignore super classes?
            outfile.fmt("{} ::{}\nend\n\n", defType, rbiEntry.show(rbiGS));
        }

        // TODO(jez) Unclear how to recurse when next source thing does not exist
        // serializeClasses(sourceGS, rbiGS, outfile, sourceChildren, rbiChildren, sourceEntryIt->second, rbiEntry);
    }
}

} // namespace

void Minimize::writeDiff(const core::GlobalState &sourceGS, const core::GlobalState &rbiGS,
                         options::PrinterConfig &outfile) {
    auto sourceChildren = core::Symbols::root().data(sourceGS)->members();
    auto rbiChildren = core::Symbols::root().data(rbiGS)->members();
    // TODO(jez) Use sed to post-process this to `typed: autogenerated` when you get to rolling this
    // out in pay-server. Also add a "do not edit" preamble.
    outfile.fmt("# typed: true\n\n");

    serializeClasses(sourceGS, rbiGS, outfile, sourceChildren, rbiChildren, core::Symbols::root(),
                     core::Symbols::root());
}

} // namespace sorbet::realmain
