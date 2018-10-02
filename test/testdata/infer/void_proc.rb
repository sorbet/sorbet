# typed: true

extend T::Helpers

sig {params(blk: T.proc.void).void}
def foo(&blk)
end

foo do
  3
end
