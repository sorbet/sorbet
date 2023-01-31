# frozen_string_literal: true
# typed: false

module T::Props::Constructor
  include T::Props::WeakConstructor
end

module T::Props::Constructor::DecoratorMethods
  extend T::Sig

  # Override what `WeakConstructor` does in order to raise errors on nils instead of ignoring them.
  private def generate_initialize_source
    T::Props::Private::InitializerGenerator.generate(props, weak: false)
  end
end
