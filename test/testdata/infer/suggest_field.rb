# typed: strict

class A
  extend T::Sig

  sig {returns(Integer)}
  def returns_integer; 0; end

  sig {void}
  def initialize
    @x = 0
  # ^^ error: The instance variable `@x` must be declared using `T.let` when specifying `# typed: strict`
    @y = begin; end
  # ^^ error: The instance variable `@y` must be declared using `T.let` when specifying `# typed: strict`
    @z = returns_integer
  # ^^ error: The instance variable `@z` must be declared using `T.let` when specifying `# typed: strict`
  end

  sig {void}
  def foo
    @foo = 0
  # ^^^^ error: The instance variable `@foo` must be declared using `T.let` when specifying `# typed: strict`
  end

  sig {returns(Integer)}
  def or_eq
    # There are two errors reported here because this looks like both a read and a write after Desugar
    @or_eq ||= 0
  # ^^^^^^ error: Use of undeclared variable `@or_eq`
  # ^^^^^^ error: The instance variable `@or_eq` must be declared using `T.let` when specifying `# typed: strict`
  end

  before do
    @before = 0
  # ^^^^^^^ error: The instance variable `@before` must be declared using `T.let` when specifying `# typed: strict`
  end
end
