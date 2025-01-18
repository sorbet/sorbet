# typed: true

class A
  extend T::Sig

  sig {params(blk:T.proc.void).void}
  def example1(&blk)
# ^ def: A#example1
  end

  sig {params(blk:T.proc.void).void}
  def example2(blk)
    example1(&blk)
    # ^ usage: A#example1
  end
end
