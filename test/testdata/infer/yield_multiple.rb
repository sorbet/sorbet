# typed: true

extend T::Helpers
sig(blk: T.proc(x: Integer, y: Symbol).returns(Integer))
  .void
def yield_two(&blk)
  blk.call(1, :hi)
  yield 1, :hi
end
