# typed: true

a, b = [1]
T.assert_type!(a, Integer)
T.assert_type!(b, NilClass)

a, *b, c = [1, 1]
T.assert_type!(a, Integer)
# b shouldn't be nilable but we aren't smart enough to specialize that
# yet.
T.assert_type!(b, T.nilable(T::Array[Integer]))
T.assert_type!(c, Integer)


a, *b, c = [1]
T.assert_type!(a, Integer)
# Similarly b should have fewer nilables
T.assert_type!(b, T.nilable(T::Array[T.nilable(Integer)]))
T.assert_type!(c, NilClass)

a, b, c, *d = [1, 2, 3, 4, 5]
T.assert_type!(a, Integer)
T.assert_type!(b, Integer)
T.assert_type!(c, Integer)
T.assert_type!(d, T.nilable(T::Array[Integer]))

extend T::Sig

sig {params(x: T.nilable([Integer, Integer])).void}
def test_nilable(x)
  a, b = x

  T.assert_type!(a, T.nilable(Integer))
  T.assert_type!(b, T.nilable(Integer))
end
