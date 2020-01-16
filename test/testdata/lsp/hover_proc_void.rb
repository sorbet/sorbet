# typed: true

extend T::Sig

sig {params(blk: T.proc.void).void}
def proc_void(&blk)
  # ^ hover: private sig {params(blk: T.proc.void).void}
end
