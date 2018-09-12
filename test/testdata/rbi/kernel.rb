# typed: true

T.assert_type!(Kernel.loop, Enumerator[T.untyped])

T.assert_type!(Kernel.loop {break}, NilClass)

# Kernel is implied
T.assert_type!(loop {break}, NilClass)
