# typed: true

extend T::Sig

a = T.let(lambda { "a" }, T.proc.returns(String))
T.assert_type!(a, T.proc.returns(Object))

b = T.let(lambda { |x| }, T.proc.params(x: Object).void)
T.assert_type!(b, T.proc.params(x: Integer).void)
