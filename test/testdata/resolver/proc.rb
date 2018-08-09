# typed: strict

class TestProc
  extend T::Helpers

  sig(blk: T.proc(i: Integer).returns(Integer))
  .returns(Integer)
  def good1(&blk)
    0
  end

  sig(blk: T.proc(a: T::Array[String]).returns(T::Array[Integer]))
  .returns(Integer)
  def good2(&blk)
    0
  end

  sig(
    x: T.proc, # error: Malformed T.proc: You must specify a return type
    y: T.proc(0).returns(Integer), # error: Too many arguments provided for method `T.proc`. Expected: `0`, got: `1`
    z: T.proc({x: Integer}).returns(0), # error: Unsupported type syntax
    w: T.proc(x: :f).returns(0), # error: Unsupported type syntax
  ).returns(NilClass)
  def bad(x, y, z, w)
  end

  sig.sig.returns(Integer) # error: Malformed `sig`: Found multiple argument lists
  def foo; 0; end

  sig(
    x: T.proc(x: Integer).sig(x: Integer).returns(Integer) # error: Malformed `proc`: Found multiple argument lists
  ).returns(Integer)
  def foo1(x); 1; end
end
