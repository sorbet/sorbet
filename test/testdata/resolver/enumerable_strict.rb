# typed: strict
class A # error: Type `Elem` declared by parent `Enumerable` must be declared again
  include Enumerable
  extend T::Sig

  sig {void}
  def each
  end
end
