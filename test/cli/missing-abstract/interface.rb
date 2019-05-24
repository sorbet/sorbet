# typed: true
module Interface
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo
  end
end
