# typed: true

a = 1
if true
  a = a
  T.assert_type!(a, Integer)
end
