# typed: true

class Foo
  extend T::Sig

  sig {returns(Integer)}
  def foo
    # ^ hover: sig { returns(Integer) }
    # ^ hover: def foo; end
    42
  end
end
