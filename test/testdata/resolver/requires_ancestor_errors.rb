# typed: true

module Helper1
  requires_ancestor Object # error: Method `requires_ancestor` does not exist on `T.class_of(Helper1)`
end

module Helper2
  extend T::Helpers

  requires_ancestor # error: Not enough arguments provided for method `T::Helpers#requires_ancestor`. Expected: `1+`, got: `0`
end

module Helper3
  extend T::Helpers

  requires_ancestor NotFound # error: Unable to resolve constant `NotFound`
end
