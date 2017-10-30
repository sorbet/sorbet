#include "Names.h"
#include "Context.h"
#include "Hashing.h"
#include <numeric> // accumulate

using namespace std;

namespace ruby_typer {
namespace ast {

ruby_typer::ast::Name::~Name() noexcept {
    if (kind == NameKind::UNIQUE)
        unique.~UniqueName();
}

unsigned int Name::hashNames(vector<NameRef> &lhs, GlobalState &ctx) {
    return accumulate(lhs.begin(), lhs.end(), 0,
                      [&ctx](int acc, NameRef &necc) -> int { return mix(acc, necc.id()); }) *
               8 +
           lhs.size();
}

unsigned int Name::hash(GlobalState &ctx) const {
    // TODO: use https://github.com/Cyan4973/xxHash
    // !!! keep this in sync with GlobalState.enter*
    switch (kind) {
        case UTF8:
            return _hash(raw.utf8);
        case UNIQUE: {
            return _hash_mix_unique((u2)unique.uniqueNameKind, UNIQUE, unique.num, unique.original.id());
        }
            DEBUG_ONLY(default : Error::raise("Unknown name kind?", kind);)
    }
}

string Name::toString(GlobalState &ctx) const {
    if (kind == UTF8) {
        return raw.utf8.toString();
    } else if (kind == UNIQUE) {
        return this->unique.original.name(ctx).toString(ctx) + "$" + to_string(this->unique.num);
    } else {
        Error::notImplemented();
    }
}

Name &NameRef::name(GlobalState &ctx) const {
    DEBUG_ONLY(Error::check(_id < ctx.names.size()));
    DEBUG_ONLY(Error::check(exists()));
    return ctx.names[_id];
}
std::string NameRef::toString(GlobalState &ctx) const {
    return name(ctx).toString(ctx);
}

} // namespace ast
} // namespace ruby_typer
