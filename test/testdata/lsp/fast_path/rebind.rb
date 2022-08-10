# typed: true

class A
  extend T::Sig

  sig {params(blk: T.proc.void).void}
  def example(&blk)
  end
end

A.new.example do
  T.reveal_type(self) # error: `T.class_of(<root>)`
end
