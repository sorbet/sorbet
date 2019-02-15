# typed: true

def _; end

class Box
  extend T::Generic
  extend T::Sig

  A = type_member

  def initialize
    @value = T.let(T.unsafe(nil), A)
  end

  sig {returns(T.any(A, Integer))}
  def read
    @value
  end
end
