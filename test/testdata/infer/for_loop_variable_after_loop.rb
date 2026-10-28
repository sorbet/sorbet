# typed: true

# https://github.com/sorbet/sorbet/issues/10265
# In Ruby, `for` loops don't define a new scope, so the loop variable
# continues to exist after the loop, holding `nil` if the collection
# was empty.

xs = T.let([1, 2, 3], T::Array[Integer])
for a in xs
  T.reveal_type(a) # error: Revealed type: `Integer`
end

T.reveal_type(a) # error: Revealed type: `T.nilable(Integer)`

# Literal arrays are tuple types; literal element types must be widened so
# that e.g. `b == 3` isn't dead code after the loop.
for b in [1, 2, 3]
end

T.reveal_type(b) # error: Revealed type: `T.nilable(Integer)`
puts("three") if b == 3

# For heterogeneous tuples, the post-loop type is the union of the
# element types, because the loop can end early via `break`, so the
# variable may hold any element (or `nil`), not specifically the first
# or the last one.
for het in [1, "one"]
  break if het.is_a?(Integer)
end

T.reveal_type(het) # error: Revealed type: `T.nilable(T.any(Integer, String))`

h = T.let({x: 1}, T::Hash[Symbol, Integer])
for k, v in h
  T.reveal_type(k) # error: Revealed type: `Symbol`
  T.reveal_type(v) # error: Revealed type: `Integer`
end

T.reveal_type(k) # error: Revealed type: `T.nilable(Symbol)`
T.reveal_type(v) # error: Revealed type: `T.nilable(Integer)`

for r in 1..3
end

T.reveal_type(r) # error: Revealed type: `T.nilable(Integer)`

# Ruby's `for` only requires the collection to respond to `each`, and the
# element type comes from the declared block parameter type of `each` --
# the only method `for` actually calls at runtime. A plain `each`-only
# class (no `Enumerable`) works as long as `each` has a signature.
class EachOnly
  extend T::Sig

  sig { params(blk: T.proc.params(arg0: Integer).void).void }
  def self.each(&blk)
    yield 1
  end
end

for e in EachOnly
  T.reveal_type(e) # error: Revealed type: `Integer`
end

T.reveal_type(e) # error: Revealed type: `T.nilable(Integer)`

# If `each` has no signature, we know nothing about what it yields, and the
# loop variable's post-loop type falls back to `T.untyped`.
class UntypedEach
  def self.each(&blk)
    yield 1
  end
end

for u in UntypedEach
end

T.reveal_type(u) # error: Revealed type: `T.untyped`

# An `each` that yields multiple values destructures into the loop
# variables the same way the block parameters would.
class PairEach
  extend T::Sig

  sig { params(blk: T.proc.params(arg0: String, arg1: Integer).void).void }
  def self.each(&blk)
    yield "one", 1
  end
end

for s, n in PairEach
  T.reveal_type(s) # error: Revealed type: `String`
  T.reveal_type(n) # error: Revealed type: `Integer`
end

T.reveal_type(s) # error: Revealed type: `T.nilable(String)`
T.reveal_type(n) # error: Revealed type: `T.nilable(Integer)`
