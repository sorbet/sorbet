#include "Names.h"
#include "Hashing.h"
#include <numeric> // accumulate
namespace ruby_typer {
namespace ast {

ruby_typer::ast::Name::~Name() noexcept {
    if (kind == NameKind::UNIQUE)
        unique.~UniqueName();
}

unsigned int Name::hashNames(std::vector<NameRef> &lhs, ContextBase &ctx) {
    return std::accumulate(lhs.begin(), lhs.end(), 0,
                           [&ctx](int acc, NameRef &necc) -> int { return mix(acc, necc.id()); }) *
               8 +
           lhs.size();
}

unsigned int Name::hash(ContextBase &ctx) const {
    // TODO: use https://github.com/Cyan4973/xxHash
    // !!! keep this in sync with ContextBase.enter*
    switch (kind) {
        case UTF8:
            return _hash(raw.utf8);
        case UNIQUE: {
            return _hash_mix_unique(unique.separator.id(), UNIQUE, unique.num, unique.original.id());
        }
            DEBUG_ONLY(default : Error::raise("Unknown name kind?", kind);)
    }
}

} // namespace ast
} // namespace ruby_typer