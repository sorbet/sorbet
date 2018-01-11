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

SymbolRef Context::selfClass() {
    Symbol &info = this->owner.info(this->state);
    if (info.isClass()) {
        return info.singletonClass(this->state);
    }
    return this->contextClass();
}

SymbolRef Context::enclosingMethod() {
    SymbolRef owner = this->owner;
    while (owner != GlobalState::defn_root() && !owner.info(this->state, false).isMethod()) {
        ENFORCE(owner.exists(), "non-existing owner in enclosingMethod");
        owner = owner.info(this->state).owner;
    }
    if (owner == GlobalState::defn_root()) {
        return GlobalState::noSymbol();
    }
    return owner;
}

SymbolRef Context::enclosingClass() {
    SymbolRef owner = this->owner;
    while (owner != GlobalState::defn_root() && !owner.info(this->state, false).isClass()) {
        ENFORCE(owner.exists(), "non-existing owner in enclosingClass");
        owner = owner.info(this->state).owner;
    }
    if (owner == GlobalState::defn_root()) {
        return GlobalState::noSymbol();
    }
    return owner;
}

SymbolRef Context::contextClass() {
    SymbolRef owner = this->owner;
    while (!owner.info(this->state, false).isClass()) {
        ENFORCE(owner.exists(), "non-existing owner in contextClass");
        owner = owner.info(this->state).owner;
    }
    return owner;
}

GlobalSubstitution::GlobalSubstitution(const GlobalState &from, GlobalState &to) {
    ENFORCE(from.symbols.size() == to.symbols.size(), "Can't substitute symbols yet");

    const_cast<GlobalState &>(from).sanityCheck();
    {
        UnfreezeFileTable unfreezeFiles(to);
        unordered_map<File *, FileRef> files;
        for (auto &f : to.files) {
            files.emplace(f.get(), FileRef(to, &f - &to.files.front()));
        }

        for (const auto &f : from.files) {
            auto it = files.find(f.get());
            if (it == files.end()) {
                fileSubstitution.push_back(to.enterFile(f->path(), f->source()));
            } else {
                FileRef ref(it->second);
                fileSubstitution.push_back(ref);
            }
        }
    }
    bool seenEmpty = false;
    {
        UnfreezeNameTable unfreezeNames(to);
        int i = 0;
        for (const Name &nm : from.names) {
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
            i++;
            ENFORCE(nameSubstitution.size() == i, "Name substitution has wrong size");
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
