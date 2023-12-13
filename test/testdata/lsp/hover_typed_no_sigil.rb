class Foo
    # ^ hover: T.class_of(Foo)

  def foo
    # ^ hover: sig { returns(T.untyped) }
    # ^ hover: def foo; end
    x = 10
  # ^ hover: This file is `# typed: false`.
  # ^ hover: Most Hover results will not appear until the file is `# typed: true` or higher.
    x
  end

  X = 1
  p(X)
  # ^ hover: Integer
end
