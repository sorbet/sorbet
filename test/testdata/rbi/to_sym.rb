# typed: strict

a = T.let("foo", T.any(String, Symbol))
T.assert_type!(a.to_sym, Symbol)
