# typed: true
extend T::Sig

sig do
  type_parameters(:Instance)
    .params(
      x: T.anything,
      mod: T::Module[T.type_parameter(:Instance)]
    )
      .returns(T.type_parameter(:Instance))
end
def assert_kind_of(x, mod)
  case x
  when mod
    return x # error: Expected `T.type_parameter(:Instance) (of Object#assert_kind_of)` but found `T.anything` for method result type
  else raise ArgumentError.new("Not an instance of #{mod}!")
  end
end

sig do
  type_parameters(:Instance)
    .params(
      x: Object,
      mod: T::Module[T.type_parameter(:Instance)]
    )
      .returns(T.type_parameter(:Instance))
end
def assert_kind_of2(x, mod)
  if x.is_a?(mod)
    return x # error: Expected `T.type_parameter(:Instance) (of Object#assert_kind_of2)` but found `Object` for method result type
  else
    raise ArgumentError.new("Not an instance of #{mod}!")
  end
end

module IFoo; end
module IBar; end
module Another; end

class Foo
  include IFoo
end

class Qux; end

sig { params(mod: T::Module[T.any(IFoo, IBar)]).void }
def example(mod)
  res = assert_kind_of(Foo.new, mod)
  T.reveal_type(res) # error: `T.any(IFoo, IBar)`

  if mod <= IFoo
    T.reveal_type(mod) # error: `T::Module[T.any(IFoo, IBar)]`
  elsif mod <= Foo
    T.reveal_type(mod) # error: `T.class_of(Foo)`
  elsif mod == IFoo
    T.reveal_type(mod) # error: `T.class_of(IFoo)`
  elsif mod <= Another
    T.reveal_type(mod) # error: `T::Module[T.any(IFoo, IBar)]`
  else
    T.reveal_type(mod) # error: `T::Module[T.any(IFoo, IBar)]`
  end
end

example(IFoo)
example(IBar)
example(Foo)
example(Qux)
#       ^^^ error: Expected `T::Module[T.any(IFoo, IBar)]` but found `T.class_of(Qux)`


