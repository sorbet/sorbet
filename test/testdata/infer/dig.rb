# typed: true
# enable-suggest-unsafe: true

extend T::Sig

class MyClass; end

sig {params(response: T::Hash[String, T::Hash[String, T::Array[T.noreturn]]]).void}
def example1(response)
  x = response.dig("", "", 0, 0)
  #                           ^ error: Unused arguments to `dig`
  T.reveal_type(x) # error: `NilClass`
end

sig {params(opts: T::Hash[String, T.any(Integer, T::Hash[String, String])]).void}
def example2(opts)
  x = opts.dig("data", "_id")
  #                    ^^^^^ error: Method `dig` does not exist on `Integer` component of `T.any(T::Hash[String, String], Integer)`
  T.reveal_type(x) # error: `T.untyped`
end

class MyClass
  def my_method; end
end

sig {params(opts: T::Array[MyClass]).void}
def example3(opts)
  x = opts.dig(0)
  T.reveal_type(x) # error: `T.nilable(MyClass)`
  x.my_method
  # ^^^^^^^^^ error: Method `my_method` does not exist on `NilClass` component of `T.nilable(MyClass)`
end

AShapeType = T.type_alias do
  {
    key: {
      nested: T::Array[{x: Integer, y: Integer}]
    },
    another: Integer,
  }
end

# Shapes are bad--ideally lot of these would be errors
sig {params(opts: AShapeType).void}
def example4(opts)
  x = opts.dig(:key, :nested)
  T.reveal_type(x) # error: `T.untyped`
  x = opts.dig(:another, :nope)
  T.reveal_type(x) # error: `T.untyped`
  x = opts.dig(:key, :nested, :x)
  T.reveal_type(x) # error: `T.untyped`
  x = opts.dig(:key, :nested, 0, :nope)
  T.reveal_type(x) # error: `T.untyped`
  x = opts.dig(:key, :nested, 0, :x)
  T.reveal_type(x) # error: `T.untyped`
end

sig {params(opts: T::Hash[String, T.any(Integer, T::Hash[String, String])]).void}
def example5(opts)
  # Right now we simply give up upon seeing keyword args. Maybe in the future
  # we should do something smarter, like have `**nil` in the `dig` RBIs, and
  # instruct Sorbet to how to typecheck methods with `**nil`.
  x = opts.dig("data", _id: 0)
  T.reveal_type(x) # error: `T.untyped`
end

MyStruct = Struct.new(:foo, :bar)

sig {params(opts: T::Hash[String, MyStruct]).void}
def example6(opts)
  # The implementation right now doesn't do anything special for `Struct`'s,
  # so it just uses the `T.untyped` on the RBI's method. We could do better
  # here, but that's a job for another day.

  x = opts.dig("data", "foo")
  T.reveal_type(x) # error: `T.untyped`

  x = opts.dig("data", :foo)
  T.reveal_type(x) # error: `T.untyped`

  x = opts.dig("data", :foo, :extra)
  T.reveal_type(x) # error: `T.untyped`

  x = opts.dig("data", 0)
  T.reveal_type(x) # error: `T.untyped`
end
