# typed: true
x = [0, "hi", :what, 3.1415926]

T.assert_type!(x[0], Integer)
T.assert_type!(x[1], String)
T.assert_type!(x[2], Symbol)
T.assert_type!(x[3], Float)
T.assert_type!(x[4], NilClass)

i = "4".to_i # make sure ruby-typer doesn't see a Literal
x[i]

T.assert_type!(x[i], T.any(NilClass, Integer, String, Symbol, Float))

T.reveal_type(x[0, 0]) # error: `[] (0-tuple)`
T.reveal_type(x[0, -1]) # error: `NilClass`
T.reveal_type(x[-2, 1]) # error: `[Symbol(:what)] (1-tuple)`
T.reveal_type(x[1, 10]) # error: `[String("hi"), Symbol(:what), Float(3.141593)] (3-tuple)`
long_max = 9223372036854775807
T.reveal_type(x[1, long_max]) # error: `[String("hi"), Symbol(:what), Float(3.141593)] (3-tuple)`
T.reveal_type(x[-6, 1]) # error: `NilClass`
T.reveal_type(x[1, 2]) # error: `[String("hi"), Symbol(:what)] (2-tuple)`

T.reveal_type(x[1..2]) # error: `T.nilable(T::Array[T.any(Integer, String, Symbol, Float)])`

T.assert_type!([1, 2].min, Integer)
T.assert_type!([1, 2].max, Integer)
T.assert_type!([1, 2].first, Integer)
T.assert_type!([1, 2].last, Integer)
T.assert_type!([].sum, Integer)
T.assert_type!([1.1, 2.2].sum, Float)


# Empty arrays are T::Array[T.untyped]
empty = []
T.assert_type!(empty, T::Array[T.untyped])
T.assert_type!(empty[0], NilClass)
T.assert_type!(empty.first, NilClass)
empty << 1
empty << 4

# Arrays of literals decay
["foo"] + ["bar"]

T.let(T.unsafe(nil), [T::Array[Integer], 0]) # error: Unsupported literal in type syntax

T.assert_type!([1].concat([:foo]), [Integer, Symbol])
T.assert_type!([1].concat([:foo]).concat([3, 4]), [Integer, Symbol, Integer, Integer])
T.assert_type!([1].concat([9.0]), [Integer, Float])
T.assert_type!([8.0].concat(["x"]), [Float, String])
T.assert_type!([1].
                 concat(T::Array[Integer].new).
                 concat(T::Array[String].new),
               T::Array[T.any(Integer, String)])

T.reveal_type([].sample) # error: `NilClass`
T.reveal_type([1, ''].sample) # error: `T.any(Integer, String)`
T.reveal_type([0.0, 0.1, 0.2].sample) # error: `Float`
T.reveal_type([].sample(2)) # error: `[] (0-tuple)`
T.reveal_type([1, ''].sample(2)) # error: `T::Array[T.any(Integer, String)]`
T.reveal_type([0.0, 0.1, 0.2].sample(2)) # error: `T::Array[Float]`
