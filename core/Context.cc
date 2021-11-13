#include "core/Context.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "common/common.h"
#include "core/GlobalSubstitution.h"
#include "core/Types.h"
#include "core/Unfreeze.h"
#include "main/pipeline/semantic_extension/SemanticExtension.h"
#include <algorithm>
#include <string>

template class std::vector<sorbet::core::NameRef>;
template class std::vector<sorbet::core::FileRef>;
template class std::vector<sorbet::core::SymbolRef>;

using namespace std;

namespace sorbet::core {

ClassOrModuleRef MutableContext::selfClass() {
    if (this->owner.isClassOrModule()) {
        return this->owner.asClassOrModuleRef().data(this->state)->singletonClass(this->state);
    }
    return this->owner.enclosingClass(this->state);
}

bool Context::permitOverloadDefinitions(const core::GlobalState &gs, FileRef sigLoc, core::SymbolRef owner) {
    if (!owner.exists()) {
        return false;
    }
    for (auto loc : owner.locs(gs)) {
        auto &file = loc.file().data(gs);
        if ((file.isPayload() || file.isStdlib()) && owner != Symbols::root() &&
            (owner != Symbols::Object() || sigLoc.data(gs).isStdlib())) {
            return true;
        }
    }

    constexpr string_view whitelistedTest = "overloads_test.rb"sv;
    return FileOps::getFileName(sigLoc.data(gs).path()) == whitelistedTest;
}

bool Context::permitOverloadDefinitions(FileRef sigLoc) const {
    return Context::permitOverloadDefinitions(state, sigLoc, owner);
}

bool MutableContext::permitOverloadDefinitions(FileRef sigLoc) const {
    return Context::permitOverloadDefinitions(state, sigLoc, owner);
}

Context::Context(const MutableContext &other) noexcept : state(other.state), owner(other.owner), file(other.file) {}

void Context::trace(string_view msg) const {
    state.trace(msg);
}

void MutableContext::trace(string_view msg) const {
    state.trace(msg);
}

Context Context::withFile(FileRef file) const {
    return Context(state, owner, file);
}

Context Context::withOwner(SymbolRef sym) const {
    return Context(state, sym, file);
}

MutableContext MutableContext::withFile(FileRef file) const {
    return MutableContext(state, owner, file);
}

MutableContext MutableContext::withOwner(SymbolRef sym) const {
    return MutableContext(state, sym, file);
}

GlobalSubstitution::GlobalSubstitution(const GlobalState &from, GlobalState &to,
                                       const GlobalState *optionalCommonParent)
    : toGlobalStateId(to.globalStateId) {
    Timer timeit(to.tracer(), "GlobalSubstitution.new", from.creation);
    ENFORCE(toGlobalStateId != 0, "toGlobalStateId is only used for sanity checks, but should always be set.");
    ENFORCE(from.classAndModules.size() == to.classAndModules.size(), "Can't substitute symbols yet");
    ENFORCE(from.methods.size() == to.methods.size(), "Can't substitute symbols yet");
    ENFORCE(from.fields.size() == to.fields.size(), "Can't substitute symbols yet");
    ENFORCE(from.typeArguments.size() == to.typeArguments.size(), "Can't substitute symbols yet");
    ENFORCE(from.typeMembers.size() == to.typeMembers.size(), "Can't substitute symbols yet");

    from.sanityCheck();
    {
        UnfreezeFileTable unfreezeFiles(to);
        int fileIdx = 0; // Skip file 0
        while (fileIdx + 1 < from.filesUsed()) {
            fileIdx++;
            if (from.files[fileIdx]->sourceType == File::Type::NotYetRead) {
                continue;
            }
            if (fileIdx < to.filesUsed() && from.files[fileIdx].get() == to.files[fileIdx].get()) {
                continue;
            }
            ENFORCE(fileIdx >= to.filesUsed() || to.files[fileIdx]->sourceType == File::Type::NotYetRead);
            to.enterNewFileAt(from.files[fileIdx], fileIdx);
        }
    }

    fastPath = false;
    if (optionalCommonParent != nullptr) {
        if (from.namesUsedTotal() == optionalCommonParent->namesUsedTotal() &&
            from.symbolsUsedTotal() == optionalCommonParent->symbolsUsedTotal()) {
            ENFORCE(to.namesUsedTotal() >= from.namesUsedTotal());
            ENFORCE(to.symbolsUsedTotal() >= from.symbolsUsedTotal());
            fastPath = true;
        }
    }

    if (!fastPath || debug_mode) {
        {
            UnfreezeNameTable unfreezeNames(to);
            utf8NameSubstitution.reserve(from.utf8Names.size());
            constantNameSubstitution.reserve(from.constantNames.size());
            uniqueNameSubstitution.reserve(from.uniqueNames.size());
            int i = -1;
            for (const UTF8Name &nm : from.utf8Names) {
                i++;
                ENFORCE_NO_TIMER(utf8NameSubstitution.size() == i, "UTF8 name substitution has wrong size");
                utf8NameSubstitution.emplace_back(to.enterNameUTF8(nm.utf8));
                ENFORCE(!fastPath || utf8NameSubstitution.back().utf8Index() == i);
            }
            // UniqueNames and ConstantNames may reference each other, necessitating some special logic here to avoid
            // crashing. We process UniqueNames first because there are fewer of them, so fewer loop iterations require
            // this special check. Tested in `core_test.cc`.
            i = -1;
            for (const UniqueName &nm : from.uniqueNames) {
                i++;
                ENFORCE(uniqueNameSubstitution.size() == i, "Unique name substitution has wrong size");
                if (nm.original.kind() == NameKind::CONSTANT &&
                    nm.original.constantIndex() >= constantNameSubstitution.size()) {
                    // Note: Duplicate of loop body below. If you change one, change the other!
                    for (u4 i = constantNameSubstitution.size(); i <= nm.original.constantIndex(); i++) {
                        auto &cnst = from.constantNames[i];
                        ENFORCE_NO_TIMER(constantNameSubstitution.size() == i,
                                         "Constant name substitution has wrong size");
                        // N.B.: cnst may reference a UniqueName, but since names are linearizeable we should have
                        // already substituted it by now.
                        constantNameSubstitution.emplace_back(to.enterNameConstant(substitute(cnst.original)));
                    }
                }

                uniqueNameSubstitution.emplace_back(
                    to.freshNameUnique(nm.uniqueNameKind, substitute(nm.original), nm.num));
                ENFORCE(!fastPath || uniqueNameSubstitution.back().uniqueIndex() == i);
            }
            for (i = constantNameSubstitution.size(); i < from.constantNames.size(); i++) {
                ENFORCE_NO_TIMER(constantNameSubstitution.size() == i, "Constant name substitution has wrong size");
                auto &nm = from.constantNames[i];
                constantNameSubstitution.emplace_back(to.enterNameConstant(substitute(nm.original)));
                ENFORCE(!fastPath || constantNameSubstitution.back().constantIndex() == i);
            }
        }

        // Enforce that the symbol tables are the same
        for (int i = 0; i < from.classAndModules.size(); ++i) {
            ENFORCE(substitute(from.classAndModules[i].name) == from.classAndModules[i].name);
            ENFORCE(from.classAndModules[i].name == to.classAndModules[i].name);
        }
        for (int i = 0; i < from.methods.size(); ++i) {
            ENFORCE(substitute(from.methods[i].name) == from.methods[i].name);
            ENFORCE(from.methods[i].name == to.methods[i].name);
        }
        for (int i = 0; i < from.fields.size(); ++i) {
            ENFORCE(substitute(from.fields[i].name) == from.fields[i].name);
            ENFORCE(from.fields[i].name == to.fields[i].name);
        }
        for (int i = 0; i < from.typeArguments.size(); ++i) {
            ENFORCE(substitute(from.typeArguments[i].name) == from.typeArguments[i].name);
            ENFORCE(from.typeArguments[i].name == to.typeArguments[i].name);
        }
        for (int i = 0; i < from.typeMembers.size(); ++i) {
            ENFORCE(substitute(from.typeMembers[i].name) == from.typeMembers[i].name);
            ENFORCE(from.typeMembers[i].name == to.typeMembers[i].name);
        }
    }

    for (auto &extension : to.semanticExtensions) {
        extension->merge(from, to, *this);
    }

    to.sanityCheck();
}

bool GlobalSubstitution::useFastPath() const {
    return fastPath;
}

ErrorBuilder MutableContext::beginError(LocOffsets loc, ErrorClass what) const {
    return state.beginError(Loc(file, loc), what);
}
ErrorBuilder Context::beginError(LocOffsets loc, ErrorClass what) const {
    return state.beginError(Loc(file, loc), what);
}
} // namespace sorbet::core
