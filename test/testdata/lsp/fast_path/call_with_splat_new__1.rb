# typed: strict

class A
  extend T::Sig

  sig {params(x: Integer).void}
  def initialize(x)
  end
end
