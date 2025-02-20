  # typed: true
  extend T::Sig

  class A
    # A#foo docs
    def foo; end
  end

  class B
    # B#foo docs
    def foo; end
  end

  sig { params(x: T.any(A, B)).void }
  def example(x)
    x.foo
    # ^ hover: A#foo docs
    # ^ hover: B#foo docs
  end
