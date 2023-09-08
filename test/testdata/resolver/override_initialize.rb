# typed: true

class A
  extend T::Sig

  sig { override.params(x: Integer).void }
  def initialize(x)
  end
end
