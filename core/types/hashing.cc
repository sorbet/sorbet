#include "core/hashing/hashing.h"
#include "core/Types.h"

using namespace std;

namespace sorbet::core {
u4 ClassType::hash(const GlobalState &gs) const {
    u4 value = static_cast<u4>(TypePtr::Tag::ClassType);
    return mix(value, this->symbol.id());
}

u4 UnresolvedClassType::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::UnresolvedClassType);
    result = mix(result, this->scope.rawId());
    for (auto name : this->names) {
        result = mix(result, _hash(name.shortName(gs)));
    }
    return result;
}

u4 UnresolvedAppliedType::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::UnresolvedAppliedType);
    result = mix(result, this->klass.id());
    for (auto &targ : targs) {
        result = mix(result, targ.hash(gs));
    }
    return result;
}

u4 LiteralType::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::LiteralType);
    auto underlying = this->underlying(gs);
    ClassOrModuleRef undSymbol = cast_type_nonnull<ClassType>(underlying).symbol;
    result = mix(result, undSymbol.id());

    uint64_t rawValue;
    switch (literalKind) {
        case LiteralType::LiteralTypeKind::String:
        case LiteralType::LiteralTypeKind::Symbol:
            return mix(result, _hash(asName(gs).shortName(gs)));
        case LiteralType::LiteralTypeKind::Float:
            rawValue = absl::bit_cast<uint64_t>(asFloat());
            break;
        case LiteralType::LiteralTypeKind::Integer:
            rawValue = absl::bit_cast<uint64_t>(asInteger());
            break;
    }

    u4 topBits = static_cast<u4>(rawValue >> 32);
    u4 bottomBits = static_cast<u4>(rawValue & 0xFFFFFFFF);
    result = mix(result, topBits);
    result = mix(result, bottomBits);
    return result;
}

u4 TupleType::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::TupleType);
    for (auto &el : this->elems) {
        result = mix(result, el.hash(gs));
    }
    return result;
}

u4 ShapeType::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::ShapeType);
    for (auto &key : this->keys) {
        result = mix(result, key.hash(gs));
    }
    return result;
}

u4 AliasType::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::AliasType);
    return mix(result, this->symbol.rawId());
}

u4 AndType::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::AndType);
    result = mix(result, this->left.hash(gs));
    return mix(result, this->right.hash(gs));
}

u4 OrType::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::OrType);
    result = mix(result, this->left.hash(gs));
    return mix(result, this->right.hash(gs));
}

u4 TypeVar::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::TypeVar);
    return mix(result, sym.id());
}

u4 AppliedType::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::AppliedType);
    result = mix(result, this->klass.id());
    for (auto &arg : targs) {
        result = mix(result, arg.hash(gs));
    }
    return result;
}

u4 LambdaParam::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::LambdaParam);
    result = mix(result, this->definition.id());
    result = mix(result, this->upperBound.hash(gs));
    // Lowerbound might not be set.
    result = mix(result, this->lowerBound == nullptr ? 0 : this->lowerBound.hash(gs));
    return result;
}

u4 SelfTypeParam::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::SelfTypeParam);
    result = mix(result, this->definition.rawId());
    return result;
}

u4 SelfType::hash(const GlobalState &gs) const {
    return static_cast<u4>(TypePtr::Tag::SelfType);
}

u4 MetaType::hash(const GlobalState &gs) const {
    u4 result = static_cast<u4>(TypePtr::Tag::MetaType);
    return mix(result, this->wrapped.hash(gs));
}

} // namespace sorbet::core
