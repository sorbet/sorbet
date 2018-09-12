# typed: true

class Impl # error: Missing definition for abstract method
  include Interface
end

module Interface
  extend T::Helpers

  interface!

  sig.abstract
  def f; end
end
