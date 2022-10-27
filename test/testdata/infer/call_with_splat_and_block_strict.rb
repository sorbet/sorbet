# typed: strict

extend T::Sig

class A
  extend T::Sig

  sig { params(a: Integer, blk: Proc).void }
  def foo(a, &blk)
  end
end

class B < A
  extend T::Sig

  sig { params(blk: Proc).void }
  def foo(&blk)
    super(*[1], &blk)
  end
end

def no_sig(&blk) # error: The method `no_sig` does not have a `sig`
  foo(*[1], &blk)
end

sig {params(a: Integer, blk: T.nilable(T.proc.returns(String))).void}
def foo(a, &blk)
  bar(*[a], &blk)
  bar(*[a], &nil)
end

sig {params(a: Integer, blk: T.nilable(T.proc.returns(String))).void}
def bar(a, &blk)
end

sig {params(a: Integer, blk: T.proc.returns(String)).void}
def baz(a, &blk)
  foo(*[a], &blk)
  foo(*[a], &nil)
end

sig { params(a: Integer, b: String, blk: T.proc.params(arg0: Integer).returns(String)).void }
def foo_splat(a, b, &blk)
end

sig { params(blk: T.proc.params(arg0: Integer).returns(String)).void }
def bar_splat(&blk)
  foo_splat(*[1, "a"], &blk)
end

sig { params(blk: T.proc.params(arg0: Integer).returns(Integer)).void }
def baz_splat(&blk)
  foo_splat(*[1, "a"], &blk) # error: Expected `T.proc.params(arg0: Integer).returns(String)` but found `T.proc.params(arg0: Integer).returns(Integer)` for block argument
end
