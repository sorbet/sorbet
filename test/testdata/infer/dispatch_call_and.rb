# typed: true
module T1; end
module T2; end
module T3; end

module A
  extend T::Helpers

  sig {returns(T1)}
  def only_a; T.unsafe(nil); end

  sig {returns(T2)}
  def both_ab; T.unsafe(nil); end

  sig {returns(T1)}
  def differ_ab; T.unsafe(nil); end
end

module B
  extend T::Helpers

  sig {returns(T3)}
  def only_b; T.unsafe(nil); end

  sig {returns(T2)}
  def both_ab; T.unsafe(nil); end

  sig {returns(T2)}
  def differ_ab; T.unsafe(nil); end
end

module OtherA
  extend T::Helpers

  sig {returns(T1)}
  def only_a; T.unsafe(nil); end
end

extend T::Helpers

sig {params(ab: T.all(A, B)).returns(T.untyped)}
def test_one(ab)
  T.assert_type!(ab.only_a, T1)
  T.assert_type!(ab.only_b, T3)
  T.assert_type!(ab.both_ab, T2)
  T.assert_type!(ab.differ_ab, T.all(T1, T2))
end

sig {params(aba: T.all(T.any(A, B), OtherA)).returns(T.untyped)}
def test_two(aba)
  T.assert_type!(aba.only_a, T1)
end
