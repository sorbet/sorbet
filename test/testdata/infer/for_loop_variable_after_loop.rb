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

# Literal arrays are tuple types; `Tuple#first` returns a literal type,
# which must be widened so that e.g. `b == 3` isn't dead code.
for b in [1, 2, 3]
end

T.reveal_type(b) # error: Revealed type: `T.nilable(Integer)`
puts("three") if b == 3

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
