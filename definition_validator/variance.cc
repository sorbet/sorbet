#include "common/typecase.h"
#include "core/Polarity.h"
#include "core/Symbols.h"
#include "core/core.h"
#include "core/errors/resolver.h"

#include "definition_validator/variance.h"

using namespace std;

namespace sorbet::definition_validator::variance {

class VarianceValidator {
private:
    const core::Loc loc;

    VarianceValidator(const core::Loc loc) : loc(loc) {}

    void validate(const core::Context ctx, const core::Polarity polarity, const core::TypePtr &type) {
        typecase(
            type, [&](const core::ClassType &klass) {},

            [&](const core::NamedLiteralType &lit) {},

            [&](const core::SelfType &self) {},

            [&](const core::SelfTypeParam &sp) {},

            [&](const core::TypeVar &tvar) {},

            [&](const core::OrType &any) {
                validate(ctx, polarity, any.left);
                validate(ctx, polarity, any.right);
            },

            [&](const core::AndType &all) {
                validate(ctx, polarity, all.left);
                validate(ctx, polarity, all.right);
            },

            [&](const core::ShapeType &shape) {
                for (auto value : shape.values) {
                    validate(ctx, polarity, value);
                }
            },

            [&](const core::TupleType &tuple) {
                for (auto value : tuple.elems) {
                    validate(ctx, polarity, value);
                }
            },

            [&](const core::AppliedType &app) {
                auto members = app.klass.data(ctx)->typeMembers();
                auto params = app.targs;

                ENFORCE(members.size() == params.size(),
                        "types should be fully saturated, but there are {} members and {} params", members.size(),
                        params.size());

                for (int i = 0; i < members.size(); ++i) {
                    auto memberVariance = members[i].data(ctx)->variance();
                    auto typeArg = params[i];

                    // The polarity used to check the parameter is negated
                    // when the parameter is defined as ContraVariant, for
                    // example:
                    //
                    // -(-a -> +b) -> +c === (+a -> -b) -> + c
                    core::Polarity paramPolarity;
                    switch (memberVariance) {
                        case core::Variance::ContraVariant:
                            paramPolarity = core::Polarities::negatePolarity(polarity);
                            break;
                        case core::Variance::Invariant:
                            paramPolarity = core::Polarity::Neutral;
                            break;
                        case core::Variance::CoVariant:
                            paramPolarity = polarity;
                            break;
                    }

                    validate(ctx, paramPolarity, typeArg);
                }
            },

            // This is where the actual variance checks are done.
            [&](const core::LambdaParam &param) {
                auto paramData = param.definition.data(ctx);
                auto paramVariance = paramData->variance();

                if (!core::Polarities::hasCompatibleVariance(polarity, paramVariance)) {
                    if (paramData->name == core::Names::Constants::AttachedClass()) {
                        if (auto e = ctx.state.beginError(this->loc, core::errors::Resolver::AttachedClassAsParam)) {
                            e.setHeader("`{}` may only be used in an `{}` context, like `{}`", "T.attached_class",
                                        ":out", "returns");
                            e.addErrorNote("Methods marked `{}` are not subject to this constraint", "private");
                        }
                    } else {
                        if (auto e = ctx.state.beginError(this->loc, core::errors::Resolver::InvalidVariance)) {
                            auto flavor = paramData->owner.isClassOrModule() &&
                                                  paramData->owner.asClassOrModuleRef().data(ctx)->isSingletonClass(ctx)
                                              ? "type_template"
                                              : "type_member";

                            auto paramName = paramData->name.show(ctx);

                            e.setHeader("`{}` `{}` was defined as `{}` but is used in an `{}` context", flavor,
                                        paramName, core::Polarities::showVariance(paramVariance),
                                        core::Polarities::showPolarity(polarity));

                            e.addErrorLine(paramData->loc(), "`{}` `{}` defined here as `{}`", flavor, paramName,
                                           core::Polarities::showVariance(paramVariance));
                            e.addErrorNote("Methods marked `{}` are not subject to this constraint", "private");
                        }
                    }
                }
            },

            [&](const core::AliasType &alias) {
                auto aliasSym = alias.symbol.dealias(ctx);

                // This can be introduced by `module_function`, which in its
                // current implementation will alias an instance method as a
                // class method.
                if (aliasSym.isMethod()) {
                    validateMethod(ctx, polarity, aliasSym.asMethodRef());
                } else {
                    Exception::raise("Unexpected type alias: {}", type.toString(ctx));
                }
            },

            [&](const core::TypePtr &skipped) {
                Exception::raise("Unexpected type in variance checking: {}", skipped.toString(ctx));
            });
    }

public:
    static void validatePolarity(const core::Loc loc, const core::Context ctx, const core::Polarity polarity,
                                 const core::TypePtr &type) {
        VarianceValidator validator(loc);
        return validator.validate(ctx, polarity, type);
    }

    // Variance checking, parameterized on the external polarity of the method context.
    static void validateMethod(const core::Context ctx, const core::Polarity polarity, const core::MethodRef method) {
        auto methodData = method.data(ctx);
        if (methodData->flags.isPrivate) {
            // `private` methods in Ruby behave like `protected[this]` methods in Scala ("object-protected" methods).
            // Variance annotations are ignored for all `protected[this]` methods in Scala, so it's fine in Sorbet too:
            //
            // > References to the type parameters in object-private or object-protected values,
            // > types, variables, or methods of the class are not checked for their variance
            // > position. In these members the type parameter may appear anywhere without restricting
            // > its legal variance annotations.
            //
            // https://scala-lang.org/files/archive/spec/2.13/04-basic-declarations-and-definitions.html#variance-annotations
            //
            // Similarly, instance variables behave the same as private methods in Ruby.
            // (This would stop being the case if Ruby ever invented syntax like `x.@foo` to access an instance
            // variable on something other than the implicit `self` that the `@foo` syntax currently implies)
            return;
        }

        // Negate the polarity for checking arguments in a ContraVariant context.
        const core::Polarity negated = core::Polarities::negatePolarity(polarity);

        for (auto &arg : methodData->arguments) {
            if (arg.type != nullptr) {
                validatePolarity(arg.loc, ctx, negated, arg.type);
            }
        }

        if (methodData->resultType != nullptr) {
            validatePolarity(methodData->loc(), ctx, polarity, methodData->resultType);
        }
    }
};

// Validates uses of type members according to their variance.
void validateMethodVariance(const core::Context ctx, const core::MethodRef method) {
    VarianceValidator::validateMethod(ctx, core::Polarity::Positive, method);
}

} // namespace sorbet::definition_validator::variance
