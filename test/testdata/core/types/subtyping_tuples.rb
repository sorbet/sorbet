# typed: true

extend T::Sig

sig { params(x: T.any([Integer, Integer], [String, String])).void }
def bar(x)
  T.reveal_type(x) # error: Revealed type: `T.any([Integer, Integer], [String, String])`
end

bar([1, 2])
bar(["a", "b"])
bar([1, "b"]) # error: Expected `T.any([Integer, Integer], [String, String])` but found `[Integer(1), String("b")]` for argument `x`
bar(["a", 2]) # error: Expected `T.any([Integer, Integer], [String, String])` but found `[String("a"), Integer(2)]` for argument `x`

x = T.let([1, 2], T.any([Integer, Integer], [Numeric, Numeric]))
T.reveal_type(x) # error: Revealed type: `T.any([Integer, Integer], [Numeric, Numeric])`

y = T.let([1, 2], T.any([Numeric, Numeric], [Integer, Integer]))
T.reveal_type(y) # error: Revealed type: `T.any([Numeric, Numeric], [Integer, Integer])`

z = T.let([1, 2.0], T.any([Integer, Numeric], [Numeric, Integer]))
T.reveal_type(z) # error: Revealed type: `T.any([Integer, Numeric], [Numeric, Integer])`
z = [1.0, 2]
z = [1.0, 2.0] # error: Incompatible assignment to variable declared via `let`: `[Float(1.000000), Float(2.000000)]` is not a subtype of `T.any([Integer, Numeric], [Numeric, Integer])`

isf = T.let([1], [T.any(Integer, Float)])
T.reveal_type(isf) # error: Revealed type: `[T.any(Integer, Float)] (1-tuple)`

fsi = T.let([0.1], [T.any(Float, Integer)])
T.reveal_type(fsi) # error: Revealed type: `[T.any(Float, Integer)] (1-tuple)`

isf_or_fsi = rand() < 0.5 ? isf : fsi
T.reveal_type(isf_or_fsi) # error: Revealed type: `[T.any(Integer, Float)] (1-tuple)`

ii = T.let([1, 1], [Integer, Integer])

nn = T.let([1, 1], [Numeric, Numeric])

if T.unsafe(true)
  x = ii # always takes this
else
  x = nn
end

T.reveal_type(x)

x[0] = 1.0 # always mutates ii

T.reveal_type(ii[0]) # error: Revealed type: `Integer`
puts ii[0].class     # => Float
