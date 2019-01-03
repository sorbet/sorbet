# typed: strict

# no arg, no block
T.assert_type!(T::Array[Float].new.sum, T.any(Float, Integer))
T.assert_type!([Rational(1, 2)].sum, T.any(Rational, Integer))
# block, no arg
T.assert_type!([1.0].sum {|f| f.to_i}, Integer)
T.assert_type!([].sum {|f| Rational(1, 2)}, T.any(Rational, Integer))
# arg, no block
T.assert_type!([1.0].sum(1), T.any(Float, Integer))
T.assert_type!(T::Array[Complex].new.sum(1.0), T.any(Complex, Float))
T.assert_type!([Rational(1, 2)].sum(3), T.any(Rational, Integer))
# arg and block
T.assert_type!([1].sum(1.0) {|t| t.to_f}, Float)
# this should raise an error but does not:
T.assert_type!(T::Array[Float].new.sum('a') {|t| 1.0}, T.any(Float, String))

[1, 2] - [1, nil]

# errors

T::Array[Float].new.sum.nan? # error: Method `nan?` does not exist on `Integer` component of `T.any(Integer, Float)`
