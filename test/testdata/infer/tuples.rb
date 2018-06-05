# typed: true
x = [0, "hi", :what]

T.assert_type!(x[0], Integer)
T.assert_type!(x[1], String)
T.assert_type!(x[2], Symbol)
T.assert_type!(x[3], NilClass)

i = "4".to_i # make sure ruby-typer doesn't see a Literal
x[i]


# TODO: Land once we type arrays properly.
#
# T.assert_type!(x[i], T.any(NilClass, Integer, String, Symbol))
