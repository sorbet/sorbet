# typed: true

extend T::Sig
sig { params(
  a: T.proc.returns(T::Hash[Integer, String]),
  b: T.proc.params(x: String).returns(T::Hash[Integer, String]),
  c: T.proc.params(x: String, y: Integer, z: Float).returns(T::Hash[Integer, String]),
  d: T.proc.params(
    x1: String, x2: Integer, x3: Float, x4: String, x5: Integer, x6: Float, x7: String, x8: Integer, x9: Float, x10: String
  ).returns(T::Hash[Integer, String]),
  e: T.proc.params(
    x1: String, x2: Integer, x3: Float, x4: String, x5: Integer, x6: Float, x7: String, x8: Integer, x9: Float, x10: String, x11: Integer
  ).returns(T::Hash[Integer, String]),
).void }
def foo(a, b, c, d, e)
  T.reveal_type(a)
  T.reveal_type(b)
  T.reveal_type(c)
  T.reveal_type(d)
  T.reveal_type(e)
end
