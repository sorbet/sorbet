# typed: true

s = ""
a, b, c = s.partition('')

T.assert_type!(a, String)
T.assert_type!(b, String)
T.assert_type!(c, String)

x, y, z = s.rpartition('')

T.assert_type!(x, String)
T.assert_type!(y, String)
T.assert_type!(z, String)
T.assert_type!(x.freeze, String)

u = "abcdefg\0\0abc".unpack('CdAD')
T.assert_type!(u, T::Array[T.nilable(T.any(Integer, Float, String))])

w = "aãeéèbc".chars
T.assert_type!(w, T::Array[String])

# encoding overloadings
"foo".encode("encoding", "other_encoding", fallback: {})
"foo".encode("encoding", fallback: {})
"foo".encode(fallback: {})

# match
m1 = "foo".match("f")
m2 = "foo".match("f", 1)
T.assert_type(m1, T.nilable(MatchData))
T.assert_type(m2, T.nilable(MatchData))

m3 = "foo".match("f") do |m|
  T.assert_type(m, MatchData)
  [m]
end
m4 = "foo".match("f", 1) do |m|
  T.assert_type(m, MatchData)
  [m]
end
T.assert_type(m3, T.nilable(T::Array[MatchData]))
T.assert_type(m4, T.nilable(T::Array[MatchData]))
