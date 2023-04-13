# typed: strict

extend T::Sig

class A
  extend T::Sig

  sig { params(blk: Proc).void }
  def foo(&blk)
  end
end

class B < A
  extend T::Sig

  sig { params(blk: Proc).void }
  def foo(&blk)
    super(&blk)
  end
end

def no_sig(&blk) # error: The method `no_sig` does not have a `sig`
  foo(1, &blk)
end

sig {params(a: Integer, blk: T.nilable(T.proc.returns(String))).void}
def foo(a, &blk)
  bar(a, &blk)
  bar(a, &nil)
end

sig {params(a: Integer, blk: T.nilable(T.proc.returns(String))).void}
def bar(a, &blk)
end

sig {params(a: Integer, blk: T.proc.returns(String)).void}
def baz(a, &blk)
  T.reveal_type(proc(&blk)) # error: Revealed type: `T.proc.returns(T.untyped)`
  foo(a, &blk)
  foo(a, &nil)
end

sig {params(blk: T.proc.params(arg0: Integer).returns(String)).void}
def int_map(&blk)
  a = [1, 2, 3].map(&blk)
  T.reveal_type(a) # error: Revealed type: `T::Array[String]`
  ["a", "b"].map(&blk) # error: Expected `T.proc.params(arg0: String).returns(T.anything)` but found `T.proc.params(arg0: Integer).returns(String)` for block argument
end

sig {params(blk: Proc).void}
def unknown_arity(&blk)
  a = [1, 2, 3].map(&blk) # error: Cannot use a `Proc` with unknown arity as a `T.proc.params(arg0: Integer).returns(T.anything)`
  T.reveal_type(a) # error: Revealed type: `T::Array[T.untyped]`
end

a = [1, 2].map(&:to_s)
T.reveal_type(a) # error: Revealed type: `T::Array[String]`
