# typed: true

class Helpers
  extend T::Sig

  sig { returns(T::Boolean) }
  def helper?
    true
  end

end
