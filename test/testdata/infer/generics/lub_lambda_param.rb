# typed: strict

def _; end

class Box
  extend T::Generic

  A = type_member

  def initialize
    @value = T.let(_, A)
  end

  sig.returns(T.any(A, Integer))
  def read
    @value
  end
end
