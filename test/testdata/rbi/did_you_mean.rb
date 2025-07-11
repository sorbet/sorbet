# typed: strict

T.assert_type!(DidYouMean::JaroWinkler.distance('foo', 'bar'), T.any(Float, Integer))
