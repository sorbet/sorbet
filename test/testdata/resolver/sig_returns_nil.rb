# typed: true
class A
  extend T::Sig

  sig do
    params(x: nil) # error: Unsupported type syntax
      .returns(nil) # error: You probably meant `.returns(NilClass)`
  end
  def foo(x)
    0 # error: Returning value that does not conform to method result type
  end

  sig do
    params(
      blk: T.proc.params(
        x: nil # error: Unsupported type syntax
      ).returns(nil) # error: You probably meant `.returns(NilClass)`
    )
    .void
  end
  def bar(&blk)
    yield 0
    0
  end
end

T.let(nil, nil) # error: Unsupported type syntax

A.new.foo(0)
