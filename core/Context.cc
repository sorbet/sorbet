#include "Context.h"
#include "Hashing.h"
#include "Types.h"
#include "common/common.h"
#include <algorithm>
#include <sstream>
#include <string>

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
    const_cast<GlobalState &>(from).sanityCheck();
    for (const auto &f : from.files) {
        auto fnd =
            find_if(to.files.begin(), to.files.end(), [&](const auto &s) -> bool { return s->path() == f->path(); });
        if (fnd == to.files.end()) {
            fileSubstitution.push_back(to.enterFile(f->path(), f->source()));
        } else {
            FileRef ref(fnd - to.files.begin());
            fileSubstitution.push_back(ref);
        }
    }
    bool seenEmpty = false;
    int i = 0;
    for (const Name &nm : from.names) {
        if (seenEmpty) {
            switch (nm.kind) {
                case NameKind::UNIQUE:
                    nameSubstitution.push_back(
                        to.freshNameUnique(nm.unique.uniqueNameKind, substitute(nm.unique.original), nm.unique.num));
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
            nameSubstitution.push_back(0);
            seenEmpty = true;
        }
        i++;
        ENFORCE(nameSubstitution.size() == i, "Name substitution has wrong size");
    }
    i = 0;
    for (const Symbol &sym : from.symbols) {
        SymbolRef fromRef = sym.ref(from);
        if (fromRef.isSynthetic()) {
            symbolSubstitution.push_back(sym.ref(from));
        } else {
            ENFORCE(sym.owner._id < fromRef._id, "non-sythetic symbol was created earlier than an owner?");
            SymbolRef toOwner = substitute(sym.owner);
            NameRef toName = substitute(sym.name);
            SymbolRef maybeFound = toOwner.info(to).findMember(to, toName);
            if (!maybeFound.exists()) {
                maybeFound = to.enterSymbol(substitute(sym.definitionLoc), toOwner, toName, sym.flags);
                maybeFound.info(to).resultType = substitute(sym.resultType);
            }
            symbolSubstitution.push_back(maybeFound);
        }
        i++;
        ENFORCE(symbolSubstitution.size() == i, "Symbol substitution has wrong size");
    }

    seenEmpty = false;

    for (const Symbol &sym : from.symbols) {
        if (!seenEmpty) {
            seenEmpty = true;
            continue;
        }
        SymbolRef fromRef = sym.ref(from);
        SymbolRef other = substitute(fromRef);
        ENFORCE(sym.name.name(const_cast<GlobalState &>(from)).kind == other.info(to).name.name(to).kind,
                "name substitution breaks name kindness");
    }
    to.sanityCheck();
}

std::shared_ptr<Type> GlobalSubstitution::substitute(std::shared_ptr<Type> from) const {
    std::shared_ptr<Type> res;
    typecase(from.get(),
             [&](OrType *o) {
                 auto left = substitute(o->left);
                 auto right = substitute(o->right);
                 if (left.get() == o->left.get() && right.get() == o->right.get()) {
                     res = from;
                 } else {
                     res = OrType::make_shared(left, right);
                 }
             },
             [&](AndType *a) {
                 auto left = substitute(a->left);
                 auto right = substitute(a->right);
                 if (left.get() == a->left.get() && right.get() == a->right.get()) {
                     res = from;
                 } else {
                     res = AndType::make_shared(left, right);
                 }
             },
             [&](ClassType *c) {
                 auto nwKlass = substitute(c->symbol);
                 if (c->symbol == nwKlass) {
                     res = from;
                 } else {
                     res = make_shared<ClassType>(nwKlass);
                 }
             },
             [&](ShapeType *c) { Error::notImplemented(); }, [&](AliasType *c) { Error::notImplemented(); },
             [&](TupleType *c) { Error::notImplemented(); });
    return res;
}
} // namespace core
} // namespace ruby_typer
