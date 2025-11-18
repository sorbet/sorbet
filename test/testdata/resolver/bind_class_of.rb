# typed: true

class Test
  extend T::Sig

  sig {params(blk: T.proc.bind(T.class_of(Test)).void).void}
  def self.test(&blk)
  end
end

Test.test do
  T.reveal_type(self) # error: Revealed type: `T.class_of(Test)`
end

