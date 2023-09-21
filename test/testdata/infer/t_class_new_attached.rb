# typed: true
class Module; include T::Sig; end

class Class
  def my_new
    # There should be an error here, but Sorbet doesn't currently model that.
    # See the comment inside the Class_new intrinsic for why.
    new(1)
  end
end

class A
end

A.my_new

class MyClass
  sig {params(x: String).void}
  def initialize(x)
  end
end

sig {params(klass: T::Class[MyClass]).void}
def example1(klass)
  klass.new
  #        ^ error: Not enough arguments provided for method `MyClass#initialize`. Expected: `1`, got: `0`
  klass.new(0)
  #         ^ error: Expected `String` but found `Integer(0)` for argument `x`
  klass.new('', 0)
  #             ^ error: Too many arguments provided for method `MyClass#initialize`. Expected: `1`, got: `2`
  instance = klass.new('')
  T.reveal_type(instance) # error: `MyClass`
end

sig do
  type_parameters(:Instance)
    .params(klass: T::Class[T.type_parameter(:Instance)])
    .void
end
def example2(klass)
  klass.new
end

sig do
  type_parameters(:Instance)
    .params(klass: T::Class[T.all(MyClass, T.type_parameter(:Instance))])
    .void
end
def example3(klass)
  klass.new
  #        ^ error: Not enough arguments provided for method `MyClass#initialize` on `MyClass` component of `T.all(MyClass, T.type_parameter(:Instance) (of Object#example3))`. Expected: `1`, got: `0`
  klass.new(0)
  #         ^ error: Expected `String` but found `Integer(0)` for argument `x`
  klass.new('', 0)
  #             ^ error: Too many arguments provided for method `MyClass#initialize`. Expected: `1`, got: `2`
  instance = klass.new('')
  T.reveal_type(instance) # error: `T.all(MyClass, T.type_parameter(:Instance) (of Object#example3))`
end

sig do
  type_parameters(:Instance)
    .params(klass: T::Class[T.anything])
    .void
end
def example4(klass)
  klass.new
end

# That there are no errors for the MyModule cases below is a weird side effect
# of the fact that we treat all modules as having an ImplicitModuleSuperclass,
# which has BasicObject as its superclass.
#
# Arguably, we should make modules descend from <top> instead of BasicObject,
# but that would be an invasive change, and it looks like it's been this way
# ~forever, so let's not worry about fixing that now.
#
# https://github.com/sorbet/sorbet/issues/7309

module MyModule
  def example
    # missing error!
    initialize
  end
end

sig {params(klass: T::Class[MyModule], m: MyModule).void}
def example5(klass, m)
  # missing error!
  klass.new
  # error, but the wrong one (should be: `initialize` doesn't exist, etc.)
  klass.new(0)
  #         ^ error: Wrong number of arguments for constructor. Expected: `0`, got: `1`
  T.let(m, BasicObject)
end
