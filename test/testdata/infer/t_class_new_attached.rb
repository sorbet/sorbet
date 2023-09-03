# typed: true
class Module; include T::Sig; end

class Class
  def my_new
    # There should be an error here, but Sorbet doesn't currently model that.
    # If we made the Class_new intrinsic dispatch to `initialize` on
    # `<AttachedClass>` we can _almost_ get the behavior we want, but doing
    # that would require `Class::<AttachedClass>` to be `upper: BasicObject`
    # which is inconvenient.
    #
    # So we might want to do some ad-hoc defaulting in the Class_new intrinsic
    # to either dispatch to `<AttachedClass>`, or if it's `<top>`, then
    # dispatch to `BasicObject`.
    #
    # Note: normally we don't do these sorts of <top> -> BasicObject
    # conversions, because of parametricity, so this would be a little bit
    # frowned upon but maybe worth it.
    #
    # https://blog.jez.io/sorbet-parametricity/
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
  klass.new(0)
  klass.new('', 0)
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
  klass.new(0)
  klass.new('', 0)
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

# This is a weird side effect of the fact that we treat all modules as having
# an ImplicitModuleSuperclass, which has BasicObject as its superclass.
#
# Arguably, we should make modules descend from <top> instead of BasicObject,
# but that would be an invasive change, and it looks like it's been this way
# ~forever, so let's not worry about fixing that now.
module MyModule
  def example
    initialize
  end
end

sig {params(klass: T::Class[MyModule], m: MyModule).void}
def example5(klass, m)
  klass.new
  klass.new(0)
  T.let(m, BasicObject)
end
