# typed: true
x = [0, "hi", :what]

T.assert_type!(x[0], Integer)
T.assert_type!(x[1], String)
T.assert_type!(x[2], Symbol)
T.assert_type!(x[3], NilClass)

i = "4".to_i # make sure ruby-typer doesn't see a Literal
x[i]

T.assert_type!(x[i], T.any(NilClass, Integer, String, Symbol))

T.assert_type!([1, 2].min, Integer)
T.assert_type!([1, 2].max, Integer)
T.assert_type!([1, 2].first, Integer)
T.assert_type!([1, 2].last, Integer)


# Empty arrays are T::Array[T.untyped]
empty = []
T.assert_type!(empty, T::Array[T.untyped])
T.assert_type!(empty[0], NilClass) # error: unable to infer the type
T.assert_type!(empty.first, NilClass) # error: unable to infer the type
empty << 1
empty << 4

# Arrays of literals decay
["foo"] + ["bar"]

[T::Array[Integer], 0] # error: Unsupported usage of bare type
