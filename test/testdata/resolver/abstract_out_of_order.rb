# typed: strict

class Impl # error: Missing definition for abstract method
  include Interface
end

module Interface
  interface!

  sig.abstract
  def f; end
end
