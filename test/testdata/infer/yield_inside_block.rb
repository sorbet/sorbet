# typed: true
extend T::Helpers

sig {params(blk: T.proc.params(x: Integer).returns(Integer)).void}
def f(&blk)
  loop do
    # Ensure that this refers to the block argument to the method, not
    # a new argument to the block for `loop`
    yield 1
  end
end

def g
  loop do
    # And it works with no explicit &blk param.
    yield 1
    # The first `yield` creates the block parameter; make sure a
    # second one still works.
    yield 1
  end
end
