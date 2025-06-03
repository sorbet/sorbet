# typed: true

extend T::Sig

class A
  def %(other)
    self
  end

  def test
    "hi"
  end
end

sig { params(x: A).void }
def test(x)
  # We should see the completion item for `%` sorted later, as users are for the
  # most part not looking for operator completions.
  x.
  # ^ completion: test, class, ...
end # error: unexpected token


