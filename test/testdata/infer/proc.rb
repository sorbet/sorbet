# typed: true
class TestProcType
  extend T::Helpers

  sig do
    params(
      blk: T.proc.params(i: Integer).returns(String),
    ).returns(String)
  end
  def f(&blk)
    T.assert_type!(blk.call(7), String)
  end

  T.assert_type!(
    proc { |x| x*x }, T.proc.params(x: T.untyped).returns(T.untyped))
  T.assert_type!(
    proc { |x, y| x*y }, T.proc.params(x: T.untyped, y: T.untyped).returns(T.untyped))
  T.assert_type!(proc { |*x| x }, Proc)
  T.assert_type!(proc { |x:, y:| x*y }, Proc)
end
