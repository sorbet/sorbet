# typed: true

require 'sorbet-runtime'

extend T::Sig

sig {params(x: Integer, y: Integer, z: String).void}
def f(x, y:, z:)
  puts x
  puts y
  puts z
end

sig {params(x: Integer, y: Integer, z: String, w: Float).void}
def g(x, y:, z:, w:)
  puts x
  puts y
  puts z
  puts w
end

shaped_hash = {y: 3, z: "hi mom"}
f(3, shaped_hash)
f(3, **shaped_hash)
g(3, **shaped_hash, w: 2.0)

untyped_hash = T.let(shaped_hash, T.untyped)
f(3, untyped_hash)
f(3, **untyped_hash)
g(3, **untyped_hash, w: 2.0)

untyped_values_hash = T.let(shaped_hash, T::Hash[Symbol, T.untyped])
  f(3, untyped_values_hash)
  #    ^^^^^^^^^^^^^^^^^^^ error: Cannot call `Object#f` with a `Hash` keyword splat because the method has required keyword parameters
  f(3, **untyped_values_hash)
  #    ^^^^^^^^^^^^^^^^^^^^^ error: Cannot call `Object#f` with a `Hash` keyword splat because the method has required keyword parameters
  g(3, **untyped_values_hash, w: 2.0)
  #    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Cannot call `Object#g` with a `Hash` keyword splat because the method has required keyword parameters

# Ensure that constructing a kwargs hash with untyped keys works
untyped = T.unsafe(:y)
f(3, **untyped_hash, untyped => 20)

sig {params(x: Integer, y: Integer, kw_splat: BasicObject).void}
def h(x, y:, **kw_splat)
  puts x
  puts y
  puts kw_splat
end

untyped_values_hash = T.let({}, T::Hash[Symbol, T.untyped])
  h(1, y: 2, **untyped_values_hash)
  #    ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Cannot call `Object#h` with a `Hash` keyword splat because the method has required keyword parameters

sig {params(kw_splat: T::Hash[Symbol, String]).void}
def i(**kw_splat)
  puts kw_splat
end

string_values_hash = T.let({}, T::Hash[Symbol, String])
i(**string_values_hash)

integer_values_hash = T.let({}, T::Hash[Symbol, Integer])
i(**integer_values_hash)
# ^^^^^^^^^^^^^^^^^^^^^ error: Expected `T::Hash[Symbol, String]` but found `T::Hash[Symbol, Integer]` for argument `kw_splat`
