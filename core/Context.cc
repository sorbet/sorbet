#include "Context.h"
#include "Hashing.h"
#include "Types.h"
#include "Unfreeze.h"

#include "common/common.h"
#include <algorithm>
#include <sstream>
#include <string>
#include <unordered_map>

template class std::vector<ruby_typer::core::NameRef>;
template class std::vector<ruby_typer::core::FileRef>;
template class std::vector<ruby_typer::core::SymbolRef>;

using namespace std;

namespace ruby_typer {
namespace core {

SymbolRef MutableContext::selfClass() {
    Symbol &data = this->owner.data(this->state);
    if (data.isClass()) {
        return data.singletonClass(this->state);
    }
    return this->contextClass();
}

bool Context::permitOverloadDefinitions() const {
    if (!owner.exists()) {
        return false;
    }
    auto &file = owner.data(*this).definitionLoc.file.data(*this);
    constexpr char const *whitelistedTest = "overloads_test.rb";
    return file.isPayload() || ::ruby_typer::File::getFileName(file.path()) == whitelistedTest;
}

bool MutableContext::permitOverloadDefinitions() const {
    Context self(*this);
    return self.permitOverloadDefinitions();
}

SymbolRef Context::contextClass() const {
    SymbolRef owner = this->owner;
    while (!owner.data(this->state, false).isClass()) {
        ENFORCE(owner.exists(), "non-existing owner in contextClass");
        owner = owner.data(this->state).owner;
    }
    return owner;
}

Context::Context(const MutableContext &other) : state(other.state), owner(other.owner) {}

SymbolRef MutableContext::contextClass() const {
    Context self(*this);
    return self.contextClass();
}

GlobalSubstitution::GlobalSubstitution(const GlobalState &from, GlobalState &to) {
    ENFORCE(from.symbols.size() == to.symbols.size(), "Can't substitute symbols yet");

    const_cast<GlobalState &>(from).sanityCheck();
    {
        UnfreezeFileTable unfreezeFiles(to);
        int fileIdx = 0; // Skip file 0
        while (fileIdx + 1 < from.filesUsed()) {
            fileIdx++;
            if (from.files[fileIdx]->source_type == core::File::TombStone) {
                continue;
            }
            if (fileIdx < to.filesUsed() && from.files[fileIdx].get() == to.files[fileIdx].get()) {
                continue;
            }
            ENFORCE(fileIdx >= to.filesUsed() || to.files[fileIdx]->source_type == core::File::TombStone);
            to.enterFileAt(from.files[fileIdx], fileIdx);
        }
    }
    bool seenEmpty = false;
    {
        UnfreezeNameTable unfreezeNames(to);
        int i = -1;
        for (const Name &nm : from.names) {
            i++;
            ENFORCE(nameSubstitution.size() == i, "Name substitution has wrong size");
            if (seenEmpty) {
                switch (nm.kind) {
                    case NameKind::UNIQUE:
                        nameSubstitution.push_back(to.freshNameUnique(nm.unique.uniqueNameKind,
                                                                      substitute(nm.unique.original), nm.unique.num));
                        break;
                    case NameKind::UTF8:
                        nameSubstitution.push_back(to.enterNameUTF8(nm.raw.utf8));
                        break;
                    case NameKind::CONSTANT:
                        nameSubstitution.push_back(to.enterNameConstant(substitute(nm.cnst.original)));
                        break;
                    default:
                        ENFORCE(false, "NameKind missing");
                }
            } else {
                nameSubstitution.push_back(NameRef(to, 0));
                seenEmpty = true;
            }
        }
    }

    // Enforce that the symbol tables are the same
    for (int i = 0; i < from.symbols.size(); ++i) {
        ENFORCE(substitute(from.symbols[i].name) == from.symbols[i].name);
        ENFORCE(from.symbols[i].name == to.symbols[i].name);
    }

    to.sanityCheck();
}

} // namespace core
} // namespace ruby_typer
