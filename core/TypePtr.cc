#include "core/TypePtr.h"
#include "core/Hashing.h"
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
            return absl::c_all_of(shape.values, [](const TypePtr &t) { return t.isFullyDefined(); });
        }
        case Tag::TupleType: {
            auto &tuple = cast_type_nonnull<TupleType>(*this);
            return absl::c_all_of(tuple.elems, [](const TypePtr &t) { return t.isFullyDefined(); });
        }
        case Tag::AndType: {
            auto &andType = cast_type_nonnull<AndType>(*this);
            return andType.left.isFullyDefined() && andType.right.isFullyDefined();
        }
        case Tag::OrType: {
            auto &orType = cast_type_nonnull<OrType>(*this);
            return orType.left.isFullyDefined() && orType.right.isFullyDefined();
        }
        case Tag::AppliedType: {
            auto &app = cast_type_nonnull<AppliedType>(*this);
            return absl::c_all_of(app.targs, [](const TypePtr &t) { return t.isFullyDefined(); });
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
            return c.symbol == Symbols::untyped();
        }
        case Tag::OrType: {
            auto &o = cast_type_nonnull<OrType>(*this);
            return o.left.hasUntyped() || o.right.hasUntyped();
        }
        case Tag::AndType: {
            auto &a = cast_type_nonnull<AndType>(*this);
            return a.left.hasUntyped() || a.right.hasUntyped();
        }
        case Tag::AppliedType: {
            auto &app = cast_type_nonnull<AppliedType>(*this);
            return absl::c_any_of(app.targs, [](const TypePtr &t) { return t.hasUntyped(); });
        }
        case Tag::TupleType: {
            auto &tuple = cast_type_nonnull<TupleType>(*this);
            return absl::c_any_of(tuple.elems, [](const TypePtr &t) { return t.hasUntyped(); });
        }
        case Tag::ShapeType: {
            auto &shape = cast_type_nonnull<ShapeType>(*this);
            return absl::c_any_of(shape.values, [](const TypePtr &t) { return t.hasUntyped(); });
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

TypePtr TypePtr::_replaceSelfType(const GlobalState &gs, const TypePtr &receiver) const {
    switch (tag()) {
        case Tag::SelfType:
            return cast_type_nonnull<SelfType>(*this)._replaceSelfType(gs, receiver);
        case Tag::OrType:
            return cast_type_nonnull<OrType>(*this)._replaceSelfType(gs, receiver);
        case Tag::AndType:
            return cast_type_nonnull<AndType>(*this)._replaceSelfType(gs, receiver);

        case Tag::UnresolvedClassType:
        case Tag::UnresolvedAppliedType:
        case Tag::BlamedUntyped:
        case Tag::LiteralType:
        case Tag::AliasType:
        case Tag::SelfTypeParam:
        case Tag::LambdaParam:
        case Tag::ClassType:
        case Tag::ShapeType:
        case Tag::TypeVar:
        case Tag::TupleType:
        case Tag::AppliedType:
        case Tag::MetaType:
            return nullptr;
    }
}

TypePtr TypePtr::_instantiate(const GlobalState &gs, const TypeConstraint &tc) const {
    switch (tag()) {
        case Tag::TypeVar:
            return cast_type_nonnull<TypeVar>(*this)._instantiate(gs, tc);
        case Tag::TupleType:
            return cast_type_nonnull<TupleType>(*this)._instantiate(gs, tc);
        case Tag::ShapeType:
            return cast_type_nonnull<ShapeType>(*this)._instantiate(gs, tc);
        case Tag::OrType:
            return cast_type_nonnull<OrType>(*this)._instantiate(gs, tc);
        case Tag::AndType:
            return cast_type_nonnull<AndType>(*this)._instantiate(gs, tc);
        case Tag::AppliedType:
            return cast_type_nonnull<AppliedType>(*this)._instantiate(gs, tc);

        case Tag::UnresolvedClassType:
        case Tag::UnresolvedAppliedType:
        case Tag::BlamedUntyped:
        case Tag::LiteralType:
        case Tag::AliasType:
        case Tag::SelfType:
        case Tag::SelfTypeParam:
        case Tag::LambdaParam:
        case Tag::ClassType:
        case Tag::MetaType:
            return nullptr;
    }
}

TypePtr TypePtr::_instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                              const std::vector<TypePtr> &targs) const {
    switch (tag()) {
        case Tag::BlamedUntyped:
        case Tag::UnresolvedAppliedType:
        case Tag::UnresolvedClassType:
        case Tag::ClassType:
        case Tag::TypeVar:
        case Tag::LiteralType:
        case Tag::SelfTypeParam:
        case Tag::SelfType:
            return nullptr;

        case Tag::TupleType:
            return cast_type_nonnull<TupleType>(*this)._instantiate(gs, params, targs);
        case Tag::ShapeType:
            return cast_type_nonnull<ShapeType>(*this)._instantiate(gs, params, targs);
        case Tag::OrType:
            return cast_type_nonnull<OrType>(*this)._instantiate(gs, params, targs);
        case Tag::AndType:
            return cast_type_nonnull<AndType>(*this)._instantiate(gs, params, targs);
        case Tag::AppliedType:
            return cast_type_nonnull<AppliedType>(*this)._instantiate(gs, params, targs);
        case Tag::LambdaParam:
            return cast_type_nonnull<LambdaParam>(*this)._instantiate(gs, params, targs);

        case Tag::MetaType:
        case Tag::AliasType:
            Exception::raise("should never happen: _instantiate on `{}`", typeName());
    }
}

void TypePtr::_sanityCheck(const GlobalState &gs) const {
    switch (tag()) {
        case Tag::BlamedUntyped:
        case Tag::UnresolvedAppliedType:
        case Tag::UnresolvedClassType:
        case Tag::ClassType:
            return cast_type_nonnull<ClassType>(*this)._sanityCheck(gs);
        case Tag::TypeVar:
            return cast_type_nonnull<TypeVar>(*this)._sanityCheck(gs);
        case Tag::LiteralType:
            return cast_type_nonnull<LiteralType>(*this)._sanityCheck(gs);
        case Tag::SelfTypeParam:
            return cast_type_nonnull<SelfTypeParam>(*this)._sanityCheck(gs);
        case Tag::SelfType:
            return cast_type_nonnull<SelfType>(*this)._sanityCheck(gs);
        case Tag::TupleType:
            return cast_type_nonnull<TupleType>(*this)._sanityCheck(gs);
        case Tag::ShapeType:
            return cast_type_nonnull<ShapeType>(*this)._sanityCheck(gs);
        case Tag::OrType:
            return cast_type_nonnull<OrType>(*this)._sanityCheck(gs);
        case Tag::AndType:
            return cast_type_nonnull<AndType>(*this)._sanityCheck(gs);
        case Tag::AppliedType:
            return cast_type_nonnull<AppliedType>(*this)._sanityCheck(gs);
        case Tag::LambdaParam:
            return cast_type_nonnull<LambdaParam>(*this)._sanityCheck(gs);
        case Tag::MetaType:
            return cast_type_nonnull<MetaType>(*this)._sanityCheck(gs);
        case Tag::AliasType:
            return cast_type_nonnull<AliasType>(*this)._sanityCheck(gs);
    }
}

string TypePtr::toStringWithTabs(const GlobalState &gs, int tabs) const {
    switch (tag()) {
        case Tag::BlamedUntyped:
            return cast_type_nonnull<BlamedUntyped>(*this).toStringWithTabs(gs, tabs);
        case Tag::UnresolvedAppliedType:
            return cast_type_nonnull<UnresolvedAppliedType>(*this).toStringWithTabs(gs, tabs);
        case Tag::UnresolvedClassType:
            return cast_type_nonnull<UnresolvedClassType>(*this).toStringWithTabs(gs, tabs);
        case Tag::ClassType:
            return cast_type_nonnull<ClassType>(*this).toStringWithTabs(gs, tabs);
        case Tag::TypeVar:
            return cast_type_nonnull<TypeVar>(*this).toStringWithTabs(gs, tabs);
        case Tag::LiteralType:
            return cast_type_nonnull<LiteralType>(*this).toStringWithTabs(gs, tabs);
        case Tag::SelfTypeParam:
            return cast_type_nonnull<SelfTypeParam>(*this).toStringWithTabs(gs, tabs);
        case Tag::SelfType:
            return cast_type_nonnull<SelfType>(*this).toStringWithTabs(gs, tabs);
        case Tag::TupleType:
            return cast_type_nonnull<TupleType>(*this).toStringWithTabs(gs, tabs);
        case Tag::ShapeType:
            return cast_type_nonnull<ShapeType>(*this).toStringWithTabs(gs, tabs);
        case Tag::OrType:
            return cast_type_nonnull<OrType>(*this).toStringWithTabs(gs, tabs);
        case Tag::AndType:
            return cast_type_nonnull<AndType>(*this).toStringWithTabs(gs, tabs);
        case Tag::AppliedType:
            return cast_type_nonnull<AppliedType>(*this).toStringWithTabs(gs, tabs);
        case Tag::LambdaParam:
            return cast_type_nonnull<LambdaParam>(*this).toStringWithTabs(gs, tabs);
        case Tag::MetaType:
            return cast_type_nonnull<MetaType>(*this).toStringWithTabs(gs, tabs);
        case Tag::AliasType:
            return cast_type_nonnull<AliasType>(*this).toStringWithTabs(gs, tabs);
    }
}

unsigned int TypePtr::hash(const GlobalState &gs) const {
    return _hash(this->toString(gs)); // TODO: make something better
}

std::string TypePtr::show(const GlobalState &gs) const {
    switch (tag()) {
        case Tag::UnresolvedAppliedType:
            return cast_type_nonnull<UnresolvedAppliedType>(*this).show(gs);
        case Tag::UnresolvedClassType:
            return cast_type_nonnull<UnresolvedClassType>(*this).show(gs);
        case Tag::BlamedUntyped:
        case Tag::ClassType:
            return cast_type_nonnull<ClassType>(*this).show(gs);
        case Tag::TypeVar:
            return cast_type_nonnull<TypeVar>(*this).show(gs);
        case Tag::LiteralType:
            return cast_type_nonnull<LiteralType>(*this).show(gs);
        case Tag::SelfTypeParam:
            return cast_type_nonnull<SelfTypeParam>(*this).show(gs);
        case Tag::SelfType:
            return cast_type_nonnull<SelfType>(*this).show(gs);
        case Tag::TupleType:
            return cast_type_nonnull<TupleType>(*this).show(gs);
        case Tag::ShapeType:
            return cast_type_nonnull<ShapeType>(*this).show(gs);
        case Tag::OrType:
            return cast_type_nonnull<OrType>(*this).show(gs);
        case Tag::AndType:
            return cast_type_nonnull<AndType>(*this).show(gs);
        case Tag::AppliedType:
            return cast_type_nonnull<AppliedType>(*this).show(gs);
        case Tag::LambdaParam:
            return cast_type_nonnull<LambdaParam>(*this).show(gs);
        case Tag::MetaType:
            return cast_type_nonnull<MetaType>(*this).show(gs);
        case Tag::AliasType:
            return cast_type_nonnull<AliasType>(*this).show(gs);
    }
}

std::string TypePtr::showWithMoreInfo(const GlobalState &gs) const {
    switch (tag()) {
        case Tag::TupleType:
            return cast_type_nonnull<TupleType>(*this).showWithMoreInfo(gs);
        default:
            return show(gs);
    }
}

} // namespace sorbet::core
