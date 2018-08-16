# typed: strict
class TestProcType
  extend T::Helpers

  sig(
    blk: T.proc(i: Integer).returns(String),
  ).returns(String)
  def f(&blk)
    T.assert_type!(blk.call(7), String)
  end

  T.assert_type!(
    proc { |x| x*x }, T.proc(x: T.untyped).returns(T.untyped))
  T.assert_type!(
    proc { |x, y| x*y }, T.proc(x: T.untyped, y: T.untyped).returns(T.untyped))
  T.assert_type!(proc { |*x| x }, Proc)
  T.assert_type!(proc { |x:, y:| x*y }, Proc)
end
