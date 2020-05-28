# typed: strict

class A
  extend T::Sig

  sig(:final) {params(x: Integer).void}
  def initialize(x)
    @x = x
  end
end
