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

  requires_ancestor NotFound
  #                 ^^^^^^^^ error: Argument to `requires_ancestor` must be statically resolvable to a class or a module
  #                 ^^^^^^^^ error: Unable to resolve constant `NotFound`
end

class Helper4
  extend T::Helpers

  requires_ancestor Object # error: `requires_ancestor` can only be declared inside a module or an abstract class

  class << self
    extend T::Helpers
    requires_ancestor String # error: `requires_ancestor` can only be declared inside a module or an abstract class
  end
end

module Helper5
  extend T::Helpers

  requires_ancestor Object, "Object"
  #                         ^^^^^^^^ error: Argument to `requires_ancestor` must be statically resolvable to a class or a module

  requires_ancestor T.class_of(Object)
  #                 ^^^^^^^^^^^^^^^^^^ error: Argument to `requires_ancestor` must be statically resolvable to a class or a module
end

module Helper6
  extend T::Helpers

  requires_ancestor Helper6
  #                 ^^^^^^^ error: Must not pass yourself to `requires_ancestor`
end
