# typed: true
extend T::Sig

module A
  extend T::Sig

  sig {params(blk: T.proc.bind(A).void).void}
  def thing(&blk)
  end

end

module B
  extend T::Sig

  sig {params(blk: T.proc.bind(B).void).void}
  def thing(&blk)
  end
end

sig {params(x: T.all(A, B)).void}
def example_1(x)
  T.reveal_type(x) # error: `T.all(A, B)`

  x.thing do
    T.reveal_type(self) # error: `T.all(A, B)`
  end
end
