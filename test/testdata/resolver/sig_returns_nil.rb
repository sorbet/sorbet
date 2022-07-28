# typed: true
class A
  extend T::Sig

  sig do
    params(x: nil) # error: Unsupported literal in type syntax
      .returns(nil) # error: Unsupported literal in type syntax
  end
  def foo(x)
    0 # error: Expected `NilClass` but found `Integer(0)` for method result type
  end

  sig do
    params(
      blk: T.proc.params(
        x: nil
        #  ^^^ error: Unsupported literal in type syntax
        #  ^^^ error: Unexpected bare `NilClass` value found in type position
      ).returns(nil)
      #         ^^^ error: Unsupported literal in type syntax
      #         ^^^ error: Unexpected bare `NilClass` value found in type position
    )
    .void
  end
  def bar(&blk)
    yield 0 # error: Expected `NilClass` but found `Integer(0)` for argument `arg0`
    0
  end
end

T.let(nil, nil) # error: Unsupported literal in type syntax

A.new.foo(0) # error: Expected `NilClass` but found `Integer(0)` for argument `x`
