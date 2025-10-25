# typed: true
extend T::Sig

sig {
  type_parameters(:T)
    .params(mod: T.any(T::Class[T.type_parameter(:T)], T::Module[T.type_parameter(:T)]))
    .returns(T.type_parameter(:T))
}
def make_dummy_of(mod)
  T.reveal_type(mod) # error: `T::Module[T.type_parameter(:T) (of Object#make_dummy_of)]`
  case mod
  when Class
    T.reveal_type(mod) # error: `T::Class[T.type_parameter(:T) (of Object#make_dummy_of)]`
    inst = mod.new
    T.reveal_type(inst) # error: `T.type_parameter(:T) (of Object#make_dummy_of)`
    return inst
  else
    # There's no API in the Ruby standard library to make a class and pass in a
    # module to include into a newly-created class, such that we could get the
    # code in this section to typecheck. (The argument must be a superclass.)
    if T.unsafe(false)
      klass_bad = Class.new(mod)
      #                     ^^^ error: Expected `T.all(T::Class[T.anything], T.type_parameter(:Parent))` but found `T::Module[T.type_parameter(:T) (of Object#make_dummy_of)]` for argument `super_class`
    end
    klass = Class.new do
      include mod
    end
    T.reveal_type(klass) # error: `T::Class[T.untyped]`
    inst = klass.new
    T.reveal_type(inst) # error: `T.untyped`

    if T.unsafe(false)
      # This would be *wild* if we supported it, but it's probably better we don't
      klass = T::Class[mod].new {}
      #                ^^^ error: Unexpected bare `T::Module[T.type_parameter(:T) (of Object#make_dummy_of)]` value found in type position
      #                     ^^^ error: Call to method `new` on `T::Class[T.untyped]` mistakes a type for a value
    end
    return inst
  end
end

module IFoo; end
class Foo
  include IFoo
end

x = make_dummy_of(IFoo)
T.reveal_type(x) # error: `IFoo`
x = make_dummy_of(Foo)
T.reveal_type(x) # error: `Foo`
