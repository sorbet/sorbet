# typed: true

class A
  extend T::Sig

  sig {params(x: Integer, blk:T.proc.void).void}
  def example1(x, &blk)
# ^ def: A#example1
  end

  sig {params(blk:T.proc.void).void}
  def example2(blk)
    example1(10, &blk)
    # ^ usage: A#example1

    args = [10]
    example1(*args, &blk)
    # ^ usage: A#example1

    example1(*args, &:foo)
                   # ^^^^ error: Method `foo` does not exist
    # ^ usage: A#example1

  end

end
