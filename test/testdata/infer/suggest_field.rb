# typed: strict

class A
  extend T::Sig

  sig {returns(Integer)}
  def returns_integer; 0; end

  sig {void}
  def initialize
    @x = 0
  # ^^ error: Use of undeclared variable `@x`
    @y = begin; end
  # ^^ error: Use of undeclared variable `@y`
    @z = returns_integer
  # ^^ error: Use of undeclared variable `@z`
  end

  sig {void}
  def foo
    @foo = 0
  # ^^^^ error: Use of undeclared variable `@foo`
  end

  sig {returns(Integer)}
  def or_eq
    @or_eq ||= 0
  # ^^^^^^ error: Use of undeclared variable `@or_eq`
  end

  before do
    @before = 0
  # ^^^^^^^ error: Use of undeclared variable `@before`
  end
end
