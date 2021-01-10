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
f(3, untyped_values_hash) # error: Passing a hash where the specific keys are unknown to a method taking keyword arguments
f(3, **untyped_values_hash) # error: Passing a hash where the specific keys are unknown to a method taking keyword arguments
g(3, **untyped_values_hash, w: 2.0) # error: Passing a hash where the specific keys are unknown to a method taking keyword arguments
