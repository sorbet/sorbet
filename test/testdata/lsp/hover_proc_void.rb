# typed: true

extend T::Sig

sig {params(blk: T.proc.void).void}
def proc_void(&blk)
  # ^ hover: sig { params(blk: T.proc.void).void }
  # ^ hover: private def proc_void(&blk); end
end
