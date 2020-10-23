#include "core/TypePtr.h"
#include "core/Symbols.h"
#include "core/Types.h"

using namespace std;

namespace sorbet::core {

void TypePtr::deleteTagged(Tag tag, void *ptr) noexcept {
    ENFORCE(ptr != nullptr);

    switch (tag) {
        case Tag::ClassType:
            delete reinterpret_cast<ClassType *>(ptr);
            break;

        case Tag::LambdaParam:
            delete reinterpret_cast<LambdaParam *>(ptr);
            break;

        case Tag::SelfTypeParam:
            delete reinterpret_cast<SelfTypeParam *>(ptr);
            break;

        case Tag::AliasType:
            delete reinterpret_cast<AliasType *>(ptr);
            break;

        case Tag::SelfType:
            delete reinterpret_cast<SelfType *>(ptr);
            break;

        case Tag::LiteralType:
            delete reinterpret_cast<LiteralType *>(ptr);
            break;

        case Tag::TypeVar:
            delete reinterpret_cast<TypeVar *>(ptr);
            break;

        case Tag::OrType:
            delete reinterpret_cast<OrType *>(ptr);
            break;

        case Tag::AndType:
            delete reinterpret_cast<AndType *>(ptr);
            break;

        case Tag::ShapeType:
            delete reinterpret_cast<ShapeType *>(ptr);
            break;

        case Tag::TupleType:
            delete reinterpret_cast<TupleType *>(ptr);
            break;

        case Tag::AppliedType:
            delete reinterpret_cast<AppliedType *>(ptr);
            break;

        case Tag::MetaType:
            delete reinterpret_cast<MetaType *>(ptr);
            break;

        case Tag::BlamedUntyped:
            delete reinterpret_cast<BlamedUntyped *>(ptr);
            break;

        case Tag::UnresolvedClassType:
            delete reinterpret_cast<UnresolvedClassType *>(ptr);
            break;

        case Tag::UnresolvedAppliedType:
            delete reinterpret_cast<UnresolvedAppliedType *>(ptr);
            break;
    }
}

bool TypePtr::isUntyped() const {
    auto *t = cast_type<ClassType>(*this);
    return t != nullptr && t->symbol == Symbols::untyped();
}

bool TypePtr::isNilClass() const {
    auto *t = cast_type<ClassType>(*this);
    return t != nullptr && t->symbol == Symbols::NilClass();
}

bool TypePtr::isBottom() const {
    auto *t = cast_type<ClassType>(*this);
    return t != nullptr && t->symbol == Symbols::bottom();
}

int TypePtr::kind() const {
    switch (tag()) {
        case Tag::AppliedType:
            return 1;
        case Tag::BlamedUntyped:
        case Tag::UnresolvedAppliedType:
        case Tag::UnresolvedClassType:
        case Tag::ClassType:
            return 2;
        case Tag::LiteralType:
            return 3;
        case Tag::ShapeType:
            return 4;
        case Tag::TupleType:
            return 5;
        case Tag::LambdaParam:
        case Tag::SelfTypeParam:
            return 6;
        case Tag::MetaType:
            return 7;
        case Tag::TypeVar:
            return 8;
        case Tag::AliasType:
            return 9;
        case Tag::OrType:
            return 10;
        case Tag::AndType:
            return 11;
        case Tag::SelfType:
            return 12;
    }
}

std::string TypePtr::typeName() const {
    switch (tag()) {
        case Tag::AppliedType:
            return "AppliedType";
        case Tag::BlamedUntyped:
            return "BlamedUntyped";
        case Tag::UnresolvedAppliedType:
            return "UnresolvedAppliedType";
        case Tag::UnresolvedClassType:
            return "UnresolvedClassType";
        case Tag::ClassType:
            return "ClassType";
        case Tag::LiteralType:
            return "LiteralType";
        case Tag::ShapeType:
            return "ShapeType";
        case Tag::TupleType:
            return "TupleType";
        case Tag::LambdaParam:
            return "LambdaParam";
        case Tag::SelfTypeParam:
            return "SelfTypeParam";
        case Tag::MetaType:
            return "MetaType";
        case Tag::TypeVar:
            return "TypeVar";
        case Tag::AliasType:
            return "AliasType";
        case Tag::OrType:
            return "OrType";
        case Tag::AndType:
            return "AndType";
        case Tag::SelfType:
            return "SelfType";
    }
}

bool TypePtr::isFullyDefined() const {
    switch (tag()) {
        // Base cases.
        case Tag::UnresolvedAppliedType:
        case Tag::UnresolvedClassType:
        case Tag::BlamedUntyped:
        case Tag::ClassType:
        case Tag::LiteralType:
        case Tag::AliasType:
        case Tag::SelfTypeParam:
        case Tag::MetaType: // MetaType: this is kinda true but kinda false. it's false for subtyping but true for
                            // inferencer.
            return true;

        case Tag::TypeVar:
        case Tag::LambdaParam:
        case Tag::SelfType:
            return false;

        // Composite types
        case Tag::ShapeType: {
            auto &shape = cast_type_nonnull<ShapeType>(*this);
            return shape.isFullyDefined();
        }
        case Tag::TupleType: {
            auto &tuple = cast_type_nonnull<TupleType>(*this);
            return tuple.isFullyDefined();
        }
        case Tag::AndType: {
            auto &andType = cast_type_nonnull<AndType>(*this);
            return andType.isFullyDefined();
        }
        case Tag::OrType: {
            auto &orType = cast_type_nonnull<OrType>(*this);
            return orType.isFullyDefined();
        }
        case Tag::AppliedType: {
            auto &app = cast_type_nonnull<AppliedType>(*this);
            return app.isFullyDefined();
        }
    }
}

bool TypePtr::hasUntyped() const {
    switch (tag()) {
        case Tag::TypeVar:
        case Tag::LiteralType:
        case Tag::SelfType:
        case Tag::AliasType:
        case Tag::SelfTypeParam:
        case Tag::LambdaParam:
        case Tag::MetaType:
            // These cannot have untyped.
            return false;

        case Tag::BlamedUntyped:
        case Tag::UnresolvedAppliedType:
        case Tag::UnresolvedClassType:
        case Tag::ClassType: {
            auto &c = cast_type_nonnull<ClassType>(*this);
            return c.hasUntyped();
        }
        case Tag::OrType: {
            auto &o = cast_type_nonnull<OrType>(*this);
            return o.hasUntyped();
        }
        case Tag::AndType: {
            auto &a = cast_type_nonnull<AndType>(*this);
            return a.hasUntyped();
        }
        case Tag::AppliedType: {
            auto &app = cast_type_nonnull<AppliedType>(*this);
            return app.hasUntyped();
        }
        case Tag::TupleType: {
            auto &tuple = cast_type_nonnull<TupleType>(*this);
            return tuple.hasUntyped();
        }
        case Tag::ShapeType: {
            auto &shape = cast_type_nonnull<ShapeType>(*this);
            return shape.hasUntyped();
        }
    }
}

core::SymbolRef TypePtr::untypedBlame() const {
    ENFORCE(hasUntyped());
    if (auto *blamed = cast_type<BlamedUntyped>(*this)) {
        return blamed->blame;
    }
    return Symbols::noSymbol();
}

TypePtr TypePtr::getCallArguments(const GlobalState &gs, NameRef name) const {
    switch (tag()) {
        case Tag::MetaType:
        case Tag::TupleType:
        case Tag::ShapeType:
        case Tag::LiteralType: {
            auto &p = cast_type_nonnull<ProxyType>(*this);
            return p.underlying().getCallArguments(gs, name);
        }
        case Tag::OrType: {
            auto &orType = cast_type_nonnull<OrType>(*this);
            return orType.getCallArguments(gs, name);
        }
        case Tag::AndType: {
            auto &andType = cast_type_nonnull<AndType>(*this);
            return andType.getCallArguments(gs, name);
        }
        case Tag::BlamedUntyped: {
            auto &c = cast_type_nonnull<BlamedUntyped>(*this);
            return c.getCallArguments(gs, name);
        }
        case Tag::UnresolvedClassType:
        case Tag::UnresolvedAppliedType:
        case Tag::ClassType: {
            auto &c = cast_type_nonnull<ClassType>(*this);
            return c.getCallArguments(gs, name);
        }
        case Tag::AppliedType: {
            auto &app = cast_type_nonnull<AppliedType>(*this);
            return app.getCallArguments(gs, name);
        }
        case Tag::SelfType:
        case Tag::SelfTypeParam:
        case Tag::LambdaParam:
        case Tag::TypeVar:
        case Tag::AliasType: {
            Exception::raise("should never happen: getCallArguments on `{}`", typeName());
        }
    }
}

TypePtr TypePtr::_approximate(const GlobalState &gs, const TypeConstraint &tc) const {
    switch (tag()) {
        case Tag::MetaType:
            return cast_type_nonnull<MetaType>(*this)._approximate(gs, tc);
        case Tag::TypeVar:
            return cast_type_nonnull<TypeVar>(*this)._approximate(gs, tc);
        case Tag::TupleType:
            return cast_type_nonnull<TupleType>(*this)._approximate(gs, tc);
        case Tag::ShapeType:
            return cast_type_nonnull<ShapeType>(*this)._approximate(gs, tc);
        case Tag::OrType:
            return cast_type_nonnull<OrType>(*this)._approximate(gs, tc);
        case Tag::AndType:
            return cast_type_nonnull<AndType>(*this)._approximate(gs, tc);
        case Tag::AppliedType:
            return cast_type_nonnull<AppliedType>(*this)._approximate(gs, tc);

        case Tag::UnresolvedClassType:
        case Tag::UnresolvedAppliedType:
        case Tag::BlamedUntyped:
        case Tag::LiteralType:
        case Tag::AliasType:
        case Tag::SelfTypeParam:
        case Tag::SelfType:
        case Tag::LambdaParam:
        case Tag::ClassType:
            return nullptr;
    }
}

} // namespace sorbet::core
