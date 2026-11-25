# typed: strict
extend T::Sig

module M; end
class A
  extend T::Sig

  sig { params(blk: T.proc.bind(T.attached_class).void).void }
  def self.foo(&blk); end
end

sig { params(x: T.all(M, T.class_of(A))).void }
def example(x)
  x.foo do
    T.reveal_type(self) # error: Revealed type: `A`
  end
end
