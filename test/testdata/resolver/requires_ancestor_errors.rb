# typed: true
# enable-experimental-requires-ancestor: true

module Helper1
  requires_ancestor { Object } # error: Method `requires_ancestor` does not exist on `T.class_of(Helper1)`
end

module Helper2
  extend T::Helpers

  requires_ancestor # error: `requires_ancestor` requires a block parameter, but no block was passed
end

module Helper3
  extend T::Helpers

  requires_ancestor { NotFound }
  #                   ^^^^^^^^ error: Argument to `requires_ancestor` must be statically resolvable to a class or a module
  #                   ^^^^^^^^ error: Unable to resolve constant `NotFound`
end

class Helper4
  extend T::Helpers

  requires_ancestor { Object } # error: `requires_ancestor` can only be declared inside a module or an abstract class

  class << self
    extend T::Helpers
    requires_ancestor { String } # error: `requires_ancestor` can only be declared inside a module or an abstract class
  end
end

module Helper5
  extend T::Helpers

  requires_ancestor { "Object" } # error: Expected `Module` but found `String("Object")` for block result type
  #                   ^^^^^^^^ error: Argument to `requires_ancestor` must be statically resolvable to a class or a module

  requires_ancestor { T.class_of(Object) } # error: Expected `Module` but found `<Type: T.class_of(Object)>` for block result type
  #                   ^^^^^^^^^^^^^^^^^^ error: Argument to `requires_ancestor` must be statically resolvable to a class or a module
end

module Helper6
  extend T::Helpers

  requires_ancestor { Helper6 }
  #                   ^^^^^^^ error: Must not pass yourself to `requires_ancestor`
end

module Helper7
  extend T::Helpers
  include Kernel

  requires_ancestor { Kernel }
  #                   ^^^^^^ error: `Kernel` is already included by `Helper7`
end

class Helper8
  extend T::Helpers

  abstract!

  requires_ancestor { Object }
  #                   ^^^^^^ error: `Object` is already inherited by `Helper8`
end

class Helper9 < String
  extend T::Helpers

  abstract!

  requires_ancestor { String }
  #                   ^^^^^^ error: `String` is already inherited by `Helper9`
end

module Helper10
  extend T::Helpers

  requires_ancestor
# ^^^^^^^^^^^^^^^^^ error: `requires_ancestor` requires a block parameter, but no block was passed
end

module Helper11
  extend T::Helpers

  requires_ancestor (Kernel)
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Too many arguments provided for method `T::Helpers#requires_ancestor`. Expected: `0`, got: `1`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `requires_ancestor` only accepts a block
# ^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `requires_ancestor` requires a block parameter, but no block was passed
end

module Helper12
  extend T::Helpers

  requires_ancestor (Kernel) { Kernel }
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Too many arguments provided for method `T::Helpers#requires_ancestor`. Expected: `0`, got: `1`
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `requires_ancestor` only accepts a block
end
