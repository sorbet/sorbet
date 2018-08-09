# typed: strict
class TestProcType
  extend T::Helpers

  sig(
    blk: T.proc(i: Integer).returns(String),
  ).returns(String)
  def f(&blk)
    T.assert_type!(blk.call(7), String)
  end
end
