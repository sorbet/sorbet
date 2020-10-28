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
