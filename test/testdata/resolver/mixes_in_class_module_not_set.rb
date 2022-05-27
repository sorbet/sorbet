# typed: strict

module A::B
  extend T::Helpers

  mixes_in_class_methods(A) # error: Could not determine that `A` was a module, not a class
end
