#include "core/Context.h"
#include "core/Hashing.h"
#include "core/Types.h"
#include "core/Unfreeze.h"

#include "common/common.h"
#include <algorithm>
#include <string>

template class std::vector<sorbet::core::NameRef>;
template class std::vector<sorbet::core::FileRef>;
template class std::vector<sorbet::core::SymbolRef>;

using namespace std;

namespace sorbet::core {

SymbolRef MutableContext::selfClass() {
    SymbolData data = this->owner.data(this->state);
    if (data->isClass()) {
        return data->singletonClass(this->state);
    }
    return this->contextClass();
}

bool Context::permitOverloadDefinitions() const {
    if (!owner.exists()) {
        return false;
    }
    for (auto loc : owner.data(*this)->locs()) {
        auto &file = loc.file().data(*this);
        constexpr string_view whitelistedTest = "overloads_test.rb"sv;
        if ((file.isPayload() && owner != Symbols::root()) || FileOps::getFileName(file.path()) == whitelistedTest) {
            return true;
        }
    }
    return false;
}

bool MutableContext::permitOverloadDefinitions() const {
    Context self(*this);
    return self.permitOverloadDefinitions();
}

SymbolRef Context::contextClass() const {
    SymbolRef owner = this->owner;
    while (!owner.data(this->state)->isClass()) {
        ENFORCE(owner.exists(), "non-existing owner in contextClass");
        owner = owner.data(this->state)->owner;
    }
    return owner;
}

Context::Context(const MutableContext &other) noexcept : state(other.state), owner(other.owner) {}

void Context::trace(string_view msg) const {
    state.trace(msg);
}

void MutableContext::trace(string_view msg) const {
    state.trace(msg);
}

Context Context::withOwner(SymbolRef sym) const {
    Context r = Context(*this);
    r.owner = sym;
    return r;
}

SymbolRef MutableContext::contextClass() const {
    Context self(*this);
    return self.contextClass();
}

GlobalSubstitution::GlobalSubstitution(const GlobalState &from, GlobalState &to,
                                       const GlobalState *optionalCommonParent)
    : toGlobalStateId(to.globalStateId) {
    ENFORCE(toGlobalStateId != 0, "toGlobalStateId is only used for sanity checks, but should always be set.");
    ENFORCE(from.symbols.size() == to.symbols.size(), "Can't substitute symbols yet");

    const_cast<GlobalState &>(from).sanityCheck();
    {
        UnfreezeFileTable unfreezeFiles(to);
        int fileIdx = 0; // Skip file 0
        while (fileIdx + 1 < from.filesUsed()) {
            fileIdx++;
            if (from.files[fileIdx]->sourceType == File::NotYetRead) {
                continue;
            }
            if (fileIdx < to.filesUsed() && from.files[fileIdx].get() == to.files[fileIdx].get()) {
                continue;
            }
            ENFORCE(fileIdx >= to.filesUsed() || to.files[fileIdx]->sourceType == File::NotYetRead);
            to.enterNewFileAt(from.files[fileIdx], fileIdx);
        }
    }

    fastPath = false;
    if (optionalCommonParent != nullptr) {
        if (from.namesUsed() == optionalCommonParent->namesUsed() &&
            from.symbolsUsed() == optionalCommonParent->symbolsUsed()) {
            ENFORCE(to.namesUsed() >= from.namesUsed());
            ENFORCE(to.symbolsUsed() >= from.symbolsUsed());
            fastPath = true;
        }
    }

    if (!fastPath || debug_mode) {
        bool seenEmpty = false;
        {
            UnfreezeNameTable unfreezeNames(to);
            nameSubstitution.reserve(from.names.size());
            int i = -1;
            for (const Name &nm : from.names) {
                i++;
                ENFORCE(nameSubstitution.size() == i, "Name substitution has wrong size");
                if (seenEmpty) {
                    switch (nm.kind) {
                        case NameKind::UNIQUE:
                            nameSubstitution.emplace_back(to.freshNameUnique(
                                nm.unique.uniqueNameKind, substitute(nm.unique.original), nm.unique.num));
                            break;
                        case NameKind::UTF8:
                            nameSubstitution.emplace_back(to.enterNameUTF8(nm.raw.utf8));
                            break;
                        case NameKind::CONSTANT:
                            nameSubstitution.emplace_back(to.enterNameConstant(substitute(nm.cnst.original)));
                            break;
                        default:
                            ENFORCE(false, "NameKind missing");
                    }
                } else {
                    nameSubstitution.emplace_back(to, 0);
                    seenEmpty = true;
                }
                ENFORCE(!fastPath || nameSubstitution.back()._id == i);
            }
        }

        // Enforce that the symbol tables are the same
        for (int i = 0; i < from.symbols.size(); ++i) {
            ENFORCE(substitute(from.symbols[i].name) == from.symbols[i].name);
            ENFORCE(from.symbols[i].name == to.symbols[i].name);
        }
    }

    to.sanityCheck();
}

bool GlobalSubstitution::useFastPath() const {
    return fastPath;
}

} // namespace sorbet::core
