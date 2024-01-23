# frozen_string_literal: true
# typed: false

module T::Props::WeakConstructor
  include T::Props::Optional
  extend T::Sig

  # checked(:never) - O(runtime object construction)
  sig {params(hash: T::Hash[Symbol, T.untyped]).void.checked(:never)}
  def initialize(hash={})
    __t_props_generated_initialize(hash)
  end

  private def __t_props_generated_initialize(hash)
    # No-op; will be overridden if there are any props.
    #
    # To see the definition for class `Foo`, run `Foo.decorator.send(:generate_initialize_source)`
  end
end

module T::Props::WeakConstructor::DecoratorMethods
  include T::Props::HasLazilySpecializedMethods::DecoratorMethods
  extend T::Sig

  def add_prop_definition(prop, rules)
    res = super
    enqueue_lazy_method_definition!(:__t_props_generated_initialize) {generate_initialize_source}
    res
  end

  private def generate_initialize_source
    T::Props::Private::InitializerGenerator.generate(
      props,
      props_with_defaults || {},
      weak: true
    )
  end
end
