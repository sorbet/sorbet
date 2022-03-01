# typed: true

module IFoo; end
module IBar; end

class Foo; include IFoo; end
class Bar; include IBar; end

module A
  extend T::Sig

  sig {params(blk: T.proc.params(arg0: IFoo, arg1: IFoo).void).void}
  def foo(&blk)
    yield Foo.new, Foo.new
  end
end

module B
  extend T::Sig

  sig {params(blk: T.proc.params(arg0: IBar, arg1: IBar).void).void}
  def foo(&blk)
    yield Bar.new, Bar.new
  end
end

extend T::Sig
sig {params(ab: T.all(A, B)).void}
def example_1(ab)
  ab.foo do |x, y|
    T.reveal_type(x) # error: `T.all(IFoo, IBar)`
    T.reveal_type(y) # error: `T.all(IFoo, IBar)`
  end
end

sig {params(ba: T.all(B, A)).void}
def example_2(ba)
  ba.foo do |x, y|
    T.reveal_type(x) # error: `T.all(IBar, IFoo)`
    T.reveal_type(y) # error: `T.all(IBar, IFoo)`
  end
end
