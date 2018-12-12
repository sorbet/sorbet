# typed: true

class Impl # error: Missing definition for abstract method
  include Interface
end

module Interface
  extend T::Sig

  interface!

  sig {abstract.void}
  def f; end
end
