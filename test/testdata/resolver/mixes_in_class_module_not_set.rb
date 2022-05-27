# typed: strict

module A::B
  extend T::Helpers

  mixes_in_class_methods(A) # error: `A` is declared implicitly, but must be defined as a `module` explicitly
end
