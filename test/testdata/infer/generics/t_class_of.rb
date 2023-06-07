# typed: true

# TODO(jez) Another consideration for all of this: having the constant literal
# `A` have type `T.class_of(A)[T.untyped]` is kind of like allowing `A` to
# simply not implement an abstract type, which is just like how it's bad that
# singleton class methods can be abstract.
#
# Maybe we should require non-fixed, invariant type members to be marked
# `abstract!`?
#

class Module
  include T::Sig
end

class A
  extend T::Sig
  extend T::Generic
  abstract!

  X = type_template

  sig {abstract.returns(X)}
  def self.foo; end

  sig {returns(Integer)}
  def instance_method; 0; end
end

class ChildA < A
  extend T::Sig
  extend T::Generic

  X = type_template

  sig {override.returns(X)}
  def self.foo; raise; end
end

class ChildAFixedString < A
  extend T::Sig
  extend T::Generic

  X = type_template {{fixed: String}}

  sig {override.returns(X)}
  def self.foo; ''; end
end


sig {params(klass: T.class_of(A)[A, Integer]).void}
def example0(klass)
  x = klass.foo
  T.reveal_type(x) # error: `Integer`
  a = klass.new
  T.reveal_type(a) # error: `A`
end


sig do
  params(klass: T.class_of(A)[Integer])
    #                         ^^^^^^^ error: Wrong number of type parameters for `T.class_of(A)`
    #                         ^^^^^^^ error: Wrong number of type parameters for `T.class_of(A)`
    #                         ^^^^^^^ error: `Integer` is not a subtype of upper bound of type member `::<Class:A>::<AttachedClass>`
    #                         ^^^^^^^ error: `Integer` is not a subtype of upper bound of type member `::<Class:A>::<AttachedClass>`
    .void
end
def example1(klass)
  x = klass.foo
  T.reveal_type(x) # error: `T.untyped`
  a = klass.new
  T.reveal_type(a) # error: `T.untyped`
end


sig {params(klass: T.class_of(A)[ChildA, Integer]).void}
def example2(klass)
  x = klass.foo
  T.reveal_type(x) # error: `Integer`
  a = klass.new
  T.reveal_type(a) # error: `ChildA`
end

example2(A)
#        ^ error: Expected `T.class_of(A)[ChildA, Integer]` but found `T.class_of(A)[A, T.untyped]` for argument `klass`
example2(ChildA)


sig do
  type_parameters(:U)
    .params(klass: T.class_of(A)[T.all(A, T.type_parameter(:U)), Integer])
    .returns(T.all(A, T.type_parameter(:U)))
end
def example3(klass)
  x = klass.foo
  T.reveal_type(x) # error: `Integer`
  a = klass.new
  T.reveal_type(a) # error: `T.all(A, T.type_parameter(:U) (of Object#example3))`
  y = a.instance_method
  T.reveal_type(y) # error: `Integer`
  a
end

instance = example3(A)
T.reveal_type(instance) # error: `A`
instance = example3(ChildA)
T.reveal_type(instance) # error: `ChildA`

# It's possible we want to change the printing of fixed type members to
# actually show them as applied types?
#
# It's weird because it would help understanding type check failures like this,
# but would not be a type that the user could copy out of the error message and
# write into a signature.
instance = example3(ChildAFixedString)
#                   ^^^^^^^^^^^^^^^^^ error: Expected `T.class_of(A)[T.all(A, T.type_parameter(:U)), Integer]` but found `T.class_of(ChildAFixedString)` for argument `klass`
T.reveal_type(instance) # error: `ChildAFixedString`


sig do
  type_parameters(:U)
    .params(klass: T.class_of(A)[T.type_parameter(:U), Integer])
    #                            ^^^^^^^^^^^^^^^^^^^^ error: `T.type_parameter(:<todo typeargument>)` is not a subtype of upper bound of type member `::<Class:A>::<AttachedClass>
    .returns(T.type_parameter(:U))
end
def example4(klass)
  x = klass.foo
  T.reveal_type(x) # error: `Integer`
  a = klass.new
  T.reveal_type(a) # error: `T.untyped`
  a
end


sig do
  type_parameters(:U)
    .params(klass: T.all(T.class_of(A)[T.untyped, Integer], T::Class[T.type_parameter(:U)]))
    .returns(T.type_parameter(:U))
end
def example5(klass)
  x = klass.foo
  T.reveal_type(x) # error: `Integer`
  a = klass.new
  T.reveal_type(a) # error: `T.type_parameter(:U) (of Object#example5)`
  # This error message is unfortunate; had the user been able to omit the
  # `<AttachedClass>` argument in `T.class_of`, it would have defaulted to `A`
  # and then been interesected with `T.type_parameter(:U)`.
  #
  # (The thing that's annoying is just the verbosity--if you want to use
  # T.type_parameter with `T::Class`, there's no upper bound so you don't need
  # a constraint. But with `T.class_of`, the specific `<AttachedClass>` will
  # have an upper bound, and thus require that the `T.type_parameter` be used
  # inside a `T.all` to fake a bound.).
  y = a.instance_method
  #     ^^^^^^^^^^^^^^^ error: Call to method `instance_method` on unconstrained generic type `T.type_parameter(:U) (of Object#example5)`

  T.reveal_type(y) # error: `T.untyped`
  a
end

instance = example5(A)
T.reveal_type(instance) # error: `A`
instance = example5(ChildA)
T.reveal_type(instance) # error: `ChildA`
instance = example5(ChildAFixedString)
#                   ^^^^^^^^^^^^^^^^^ error: Expected `T.class_of(A)[T.type_parameter(:U), Integer]` but found `T.class_of(ChildAFixedString)` for argument `klass`
T.reveal_type(instance) # error: `ChildAFixedString`





class B
  extend T::Sig

  sig {returns(String)}
  def self.foo; ''; end

  sig {returns(Integer)}
  def instance_method; 0; end
end

class ChildB < B
end

sig do
  type_parameters(:Instance)
    .params(klass: T.class_of(B)[T.all(T.type_parameter(:Instance), B)])
    .returns(T.type_parameter(:Instance))
end
def example6(klass)
  T.reveal_type(klass) # error: `T.class_of(B)[T.all(B, T.type_parameter(:Instance) (of Object#example6))]`
  x = klass.foo
  T.reveal_type(x) # error: `String`
  b = klass.new
  T.reveal_type(b) # error: `T.all(B, T.type_parameter(:Instance) (of Object#example6))`
  y = b.instance_method
  T.reveal_type(y) # error: `Integer`
  b
end

example6(B)
example6(ChildB)
