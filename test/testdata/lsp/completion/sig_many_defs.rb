# typed: true

# This test tests that the NextMethodFinder doesn't have an off-by-one error.

class A
  extend T::Sig

  def returns_integer
    0
  end

  def returns_string
    ''
  end

  sig # error: no block
  #  ^ apply-completion: [A] item: 0

  def returns_symbol
    :''
  end

  def returns_float
    0.0
  end
end
