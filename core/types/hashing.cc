#include "core/hashing/hashing.h"
#include "core/Types.h"

using namespace std;

namespace sorbet::core {
uint32_t ClassType::hash(const GlobalState &gs) const {
    uint32_t value = static_cast<uint32_t>(TypePtr::Tag::ClassType);
    return mix(value, this->symbol.id());
}

uint32_t UnresolvedClassType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::UnresolvedClassType);
    result = mix(result, this->scope.rawId());
    for (auto name : this->names) {
        result = mix(result, _hash(name.shortName(gs)));
    }
    return result;
}

uint32_t UnresolvedAppliedType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::UnresolvedAppliedType);
    result = mix(result, this->klass.id());
    for (auto &targ : targs) {
        result = mix(result, targ.hash(gs));
    }
    return result;
}

uint32_t NamedLiteralType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::NamedLiteralType);
    auto underlying = this->underlying(gs);
    ClassOrModuleRef undSymbol = cast_type_nonnull<ClassType>(underlying).symbol;
    result = mix(result, undSymbol.id());

    return mix(result, _hash(name.shortName(gs)));
}

uint32_t FloatLiteralType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::FloatLiteralType);
    auto underlying = this->underlying(gs);
    ClassOrModuleRef undSymbol = cast_type_nonnull<ClassType>(underlying).symbol;
    result = mix(result, undSymbol.id());

    uint64_t rawValue = absl::bit_cast<uint64_t>(this->value);
    uint32_t topBits = static_cast<uint32_t>(rawValue >> 32);
    uint32_t bottomBits = static_cast<uint32_t>(rawValue & 0xFFFFFFFF);
    result = mix(result, topBits);
    result = mix(result, bottomBits);
    return result;
}

uint32_t IntegerLiteralType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::IntegerLiteralType);
    auto underlying = this->underlying(gs);
    ClassOrModuleRef undSymbol = cast_type_nonnull<ClassType>(underlying).symbol;
    result = mix(result, undSymbol.id());

    uint64_t rawValue = this->value;
    uint32_t topBits = static_cast<uint32_t>(rawValue >> 32);
    uint32_t bottomBits = static_cast<uint32_t>(rawValue & 0xFFFFFFFF);
    result = mix(result, topBits);
    result = mix(result, bottomBits);
    return result;
}

uint32_t TupleType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::TupleType);
    for (auto &el : this->elems) {
        result = mix(result, el.hash(gs));
    }
    return result;
}

uint32_t ShapeType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::ShapeType);
    for (auto &key : this->keys) {
        result = mix(result, key.hash(gs));
    }
    return result;
}

uint32_t AliasType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::AliasType);
    return mix(result, this->symbol.rawId());
}

uint32_t AndType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::AndType);
    result = mix(result, this->left.hash(gs));
    return mix(result, this->right.hash(gs));
}

uint32_t OrType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::OrType);
    result = mix(result, this->left.hash(gs));
    return mix(result, this->right.hash(gs));
}

uint32_t TypeVar::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::TypeVar);
    return mix(result, sym.id());
}

uint32_t AppliedType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::AppliedType);
    result = mix(result, this->klass.id());
    for (auto &arg : targs) {
        result = mix(result, arg.hash(gs));
    }
    return result;
}

uint32_t LambdaParam::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::LambdaParam);
    result = mix(result, this->definition.id());
    result = mix(result, this->upperBound.hash(gs));
    // Lowerbound might not be set.
    result = mix(result, this->lowerBound == nullptr ? 0 : this->lowerBound.hash(gs));
    return result;
}

uint32_t SelfTypeParam::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::SelfTypeParam);
    result = mix(result, this->definition.rawId());
    return result;
}

uint32_t NewSelfType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::NewSelfType);
    return mix(result, this->upperBound.hash(gs));
}

uint32_t MetaType::hash(const GlobalState &gs) const {
    uint32_t result = static_cast<uint32_t>(TypePtr::Tag::MetaType);
    return mix(result, this->wrapped.hash(gs));
}

} // namespace sorbet::core
