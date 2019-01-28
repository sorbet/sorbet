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
