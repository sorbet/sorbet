# typed: true

module Foo
  extend T::Sig

  sig {params(blk: T.proc.void).void}
  def foo(&blk); end

  sig {params(blk: T.proc.void).void}
  def baz(&blk)
    foo(&blk)
  # ^ hover: sig {params(blk: T.proc.void).void}
    foo {}
  # ^ hover: sig {params(blk: T.proc.void).void}
  end
end
