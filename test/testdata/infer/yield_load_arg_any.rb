# typed: true
class A
  extend T::Sig

  sig {params(blk: T.proc.params(arg0: Integer, arg1: Integer).void).void}
  def foo(&blk)
    yield 0, 0
  end
end

class B
  extend T::Sig

  sig {params(blk: T.proc.params(arg0: String, arg1: String).void).void}
  def foo(&blk)
    yield '', ''
  end
end

extend T::Sig
sig {params(ab: T.any(A, B)).void}
def example_1(ab)
  ab.foo do |x, y|
    T.reveal_type(x) # error: `T.any(Integer, String)`
    T.reveal_type(y) # error: `T.any(Integer, String)`
  end
end

sig {params(ba: T.any(B, A)).void}
def example_2(ba)
  ba.foo do |x, y|
    T.reveal_type(x) # error: `T.any(String, Integer)`
    T.reveal_type(y) # error: `T.any(String, Integer)`
  end
end
