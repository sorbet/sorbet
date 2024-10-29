#include "core/Context.h"
#include "core/GlobalState.h"
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

Loc Context::locAt(LocOffsets offsets) const {
    return Loc{this->file, offsets};
}

Loc MutableContext::locAt(LocOffsets offsets) const {
    return Loc{this->file, offsets};
}

ErrorBuilder MutableContext::beginError(LocOffsets loc, ErrorClass what) const {
    return state.beginError(Loc(file, loc), what);
}
ErrorBuilder MutableContext::beginIndexerError(LocOffsets loc, ErrorClass what) const {
        if (what.code < 4000) {
        // As errors from the indexing phase control whether or not we should cache trees, we set this flag on the file
        // even if the erorr would be suppressed, to ensure that the experience when the cache is enabled is consistent.
        this->file.data(*this).setHasIndexErrors(true);

        // All parse errors are index errors, but not all index errors are parse errors.
        if (what.code < 3000) {
            this->file.data(*this).setHasParseErrors(true);
        }
    }

    return this->beginError(loc, what);
}
ErrorBuilder Context::beginError(LocOffsets loc, ErrorClass what) const {
    return state.beginError(Loc(file, loc), what);
}
} // namespace sorbet::core
