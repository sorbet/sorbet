#include "core/hashing/hashing.h"
#include "core/SymbolRef.h"
#include "core/Types.h"

using namespace std;

namespace sorbet::core {
void ClassType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::ClassType));
    hasher.mixUint(this->symbol.id());
}

void UnresolvedClassType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::UnresolvedClassType));
    hasher.mixUint(this->scope.rawId());
    for (auto name : this->names) {
        hasher.mixString(name.shortName(gs));
    }
}

void UnresolvedAppliedType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::UnresolvedAppliedType));
    hasher.mixUint(this->klass.rawId());
    for (auto &targ : targs) {
        targ.hash(gs, hasher);
    }
}

void LiteralType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::LiteralType));
    auto underlying = this->underlying(gs);
    ClassOrModuleRef undSymbol = cast_type_nonnull<ClassType>(underlying).symbol;
    hasher.mixUint(undSymbol.id());

    switch (literalKind) {
        case LiteralType::LiteralTypeKind::String:
        case LiteralType::LiteralTypeKind::Symbol:
            return hasher.mixString(asName(gs).shortName(gs));
        case LiteralType::LiteralTypeKind::Float:
            return hasher.mixU8(absl::bit_cast<u8>(asFloat()));
        case LiteralType::LiteralTypeKind::Integer:
            return hasher.mixU8(absl::bit_cast<u8>(asInteger()));
    }
}

void TupleType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::TupleType));
    for (auto &el : this->elems) {
        el.hash(gs, hasher);
    }
}

void ShapeType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::ShapeType));
    for (auto &key : this->keys) {
        key.hash(gs, hasher);
    }
}

void AliasType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::AliasType));
    hasher.mixUint(this->symbol.rawId());
}

void AndType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::AndType));
    this->left.hash(gs, hasher);
    this->right.hash(gs, hasher);
}

void OrType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::OrType));
    this->left.hash(gs, hasher);
    this->right.hash(gs, hasher);
}

void TypeVar::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::TypeVar));
    hasher.mixUint(sym.rawId());
}

void AppliedType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::AppliedType));
    hasher.mixUint(this->klass.id());
    for (auto &arg : targs) {
        arg.hash(gs, hasher);
    }
}

void LambdaParam::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::LambdaParam));
    hasher.mixUint(this->definition.rawId());
    this->upperBound.hash(gs, hasher);
    // Lowerbound might not be set.
    if (this->lowerBound != nullptr) {
        this->lowerBound.hash(gs, hasher);
    } else {
        hasher.mixUint(0);
    }
}

void SelfTypeParam::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::SelfTypeParam));
    hasher.mixUint(this->definition.rawId());
}

void SelfType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::SelfType));
}

void MetaType::hash(const GlobalState &gs, Hasher &hasher) const {
    hasher.mixUint(static_cast<u4>(TypePtr::Tag::MetaType));
    this->wrapped.hash(gs, hasher);
}

} // namespace sorbet::core
