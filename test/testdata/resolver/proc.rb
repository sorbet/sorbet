# typed: true

class TestProc
  extend T::Sig

  sig do
    params(blk: T.proc.params(i: Integer).returns(Integer))
    .returns(Integer)
  end
  def good1(&blk)
    0
  end

  sig do
    params(blk: T.proc.params(a: T::Array[String]).returns(T::Array[Integer]))
    .returns(Integer)
  end
  def good2(&blk)
    0
  end

  sig do
    params(
      x: T.proc, # error: Malformed T.proc: You must specify a return type
      y: T.proc(0).returns(Integer),
      #         ^ error: Too many arguments provided for method `T.proc`. Expected: `0`, got: `1`
      z: T.proc.params({x: Integer}).returns(0),
      #                                      ^ error: Unsupported literal in type syntax
      #                                      ^ error: Unexpected bare `Integer(0)` value found in type position
      #  ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: `params` expects keyword arguments
      w: T.proc.params(x: :f).returns(Integer),
      #                   ^^ error: Unsupported literal in type syntax
      #                   ^^ error: Unexpected bare `Symbol(:f)` value found in type position
    ).returns(NilClass)
  end
  def bad(x, y, z, w)
  end

  sig {params(x: Integer).params(x: Integer).returns(Integer)} # error: Malformed `params`: Multiple calls to `.params`
  def foo(x); 0; end

  sig do
    params(
      x: T.proc.params(x: Integer).params(x: Integer).returns(Integer) # error: Malformed `params`: Multiple calls to `.params`
      #                            ^^^^^^ error: Call to method `params` on `T.proc.params(arg0: Integer).returns(<todo sym>)` mistakes a type for a value
    ).returns(Integer)
  end
  def foo1(x); 1; end
end
