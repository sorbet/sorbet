# typed: true
class A
  extend T::Sig
  extend T::Helpers
  abstract!
  sig {abstract.void}
  def foo; 3 end
end
