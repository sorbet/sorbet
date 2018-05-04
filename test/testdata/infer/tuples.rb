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
T.assert_type!([].min, NilClass)
T.assert_type!([1, 2].max, Integer)
T.assert_type!([].max, NilClass)

T.assert_type!([1, 2].first, Integer)
T.assert_type!([].first, NilClass)

T.assert_type!([1, 2].last, Integer)
T.assert_type!([].last, NilClass)
