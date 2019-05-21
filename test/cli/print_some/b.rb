# typed: true

class B
  extend T::Sig

  sig {returns(A)}
  def bmeth
    A.new
  end
end
