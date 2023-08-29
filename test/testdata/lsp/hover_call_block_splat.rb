# typed: true

module Foo
  extend T::Sig

  sig {params(blk: T.proc.void).void}
  def foo(&blk); end

  sig {params(blk: T.proc.void).void}
  def baz(&blk)
    foo(&blk)
  # ^ hover: sig { params(blk: T.proc.void).void }
    foo {}
  # ^ hover: sig { params(blk: T.proc.void).void }
  end

  sig {params(args: Integer).void}
  def splat_fun(*args); end

  sig {params(x: Integer).void}
  def call_splat_fun(x)
    splat_fun(*[x, x])
  # ^ hover: sig { params(args: Integer).void }
  # ^ hover: def splat_fun(*args); end
  end
end
