# typed: true

extend T::Helpers
sig do
  params(blk: T.proc.params(x: Integer, y: Symbol).returns(Integer))
    .void
end
def yield_two(&blk)
  blk.call(1, :hi)
  yield 1, :hi
end
