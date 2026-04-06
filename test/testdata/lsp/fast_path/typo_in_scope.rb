# typed: strict


module A::B
  class C
  end
end

class Other
  extend T::Sig
  sig { returns(A::B::C) }
  def call
    raise
  end
end
