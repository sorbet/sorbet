# typed: true

class B
  extend T::Sig

  sig { params(i: Integer).void }
  def foo(i)
    "lol"
  end
end
