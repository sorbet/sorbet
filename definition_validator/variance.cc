
#include "common/typecase.h"
#include "core/Symbols.h"
#include "core/core.h"
#include "core/errors/resolver.h"

#include "variance.h"

using namespace std;

namespace sorbet::definition_validator::variance {

class VarianceValidator {
private:
    const core::Loc loc;

    VarianceValidator(const core::Loc loc) : loc(loc) {}

    core::Variance invertVariance(const core::Variance variance) {
        switch (variance) {
            case core::Variance::CoVariant:
                return core::Variance::ContraVariant;
            case core::Variance::Invariant:
                return core::Variance::Invariant;
            case core::Variance::ContraVariant:
                return core::Variance::CoVariant;
        }
    }

    string showVariance(const core::Variance variance) {
        switch (variance) {
            case core::Variance::CoVariant:
                return ":out";
            case core::Variance::Invariant:
                return ":invariant";
            case core::Variance::ContraVariant:
                return ":in";
        }
    }

    void validate(const core::Context ctx, const core::Variance variance, const core::TypePtr type) {
        typecase(
            type.get(), [&](core::LiteralType *lit) {},

            [&](core::ClassType *klass) {},

            [&](core::OrType *any) {
                validate(ctx, variance, any->left);
                validate(ctx, variance, any->right);
            },

            [&](core::AndType *all) {
                validate(ctx, variance, all->left);
                validate(ctx, variance, all->right);
            },

            [&](core::ShapeType *shape) {
                for (auto value : shape->values) {
                    validate(ctx, variance, value);
                }
            },

            [&](core::TupleType *tuple) {
                for (auto value : tuple->elems) {
                    validate(ctx, variance, value);
                }
            },

            [&](core::AppliedType *app) {
                auto members = app->klass.data(ctx)->typeMembers();
                auto params = app->targs;

                // We expect that types are fully saturated.
                ENFORCE(members.size() == params.size());

                for (int i = 0; i < members.size(); ++i) {
                    auto memberVariance = members[i].data(ctx)->variance();
                    auto typeArg = params[i];

                    // Invert the variance when the type_member was defined as
                    // contravariant (:in).
                    const core::Variance checkedVariance =
                        (variance == core::Variance::ContraVariant) ? invertVariance(memberVariance) : memberVariance;

                    validate(ctx, checkedVariance, typeArg);
                }
            },

            // This is where the actual variance checks are done.
            [&](core::LambdaParam *param) {
                auto paramData = param->definition.data(ctx);

                ENFORCE(paramData->isTypeMember());

                auto paramVariance = paramData->variance();

                if (paramVariance != variance) {
                    if (auto e = ctx.state.beginError(this->loc, core::errors::Resolver::InvalidVariance)) {
                        auto flavor =
                            paramData->owner.data(ctx)->isSingletonClass(ctx) ? "type_template" : "type_member";

                        auto paramName = paramData->name.data(ctx)->show(ctx);

                        e.setHeader("`{}` `{}` was defined as `{}` but is used in an `{}` context", flavor, paramName,
                                    showVariance(paramVariance), showVariance(variance));

                        e.addErrorLine(paramData->loc(), "`{}` `{}` defined here as `{}`", flavor, paramName,
                                       showVariance(paramVariance));
                    }
                }
            },

            [&](core::MetaType *mt) { validate(ctx, variance, mt->wrapped); },

            [&](core::Type *skipped) { Exception::raise("skipped type {}", skipped->toString(ctx)); });
    }

public:
    static void validateCoVariant(const core::Loc loc, const core::Context ctx, const core::TypePtr type) {
        VarianceValidator validator(loc);
        return validator.validate(ctx, core::Variance::CoVariant, type);
    }

    static void validateContraVariant(const core::Loc loc, const core::Context ctx, const core::TypePtr type) {
        VarianceValidator validator(loc);
        return validator.validate(ctx, core::Variance::ContraVariant, type);
    }
};

// Validates uses of type members according to their variance.
void validateMethodVariance(const core::Context ctx, const core::SymbolRef method) {
    auto methodData = method.data(ctx);

    for (auto &arg : methodData->arguments()) {
        if (arg.type != nullptr) {
            VarianceValidator::validateContraVariant(arg.loc, ctx, arg.type);
        }
    }

    if (methodData->resultType != nullptr) {
        VarianceValidator::validateCoVariant(methodData->loc(), ctx, methodData->resultType);
    }
}

} // namespace sorbet::definition_validator::variance
