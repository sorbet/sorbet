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
ErrorBuilder Context::beginError(LocOffsets loc, ErrorClass what) const {
    return state.beginError(Loc(file, loc), what);
}
} // namespace sorbet::core
