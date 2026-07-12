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

# Ruby's `for` only requires the collection to respond to `each`. An
# `each`-only class (no `Enumerable`, no `first`) must not report an error;
# the loop variable's post-loop type falls back to `T.untyped`.
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

T.reveal_type(e) # error: Revealed type: `T.untyped`
