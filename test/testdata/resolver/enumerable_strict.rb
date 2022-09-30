# typed: strict

class A # error: Type `Elem` declared by parent `Enumerable` must be re-declared in `A`
  include Enumerable
  extend T::Sig

  sig {void}
  def each # error: Implementation of abstract method `Enumerable#each` must explicitly name a block argument
# ^^^^^^^^ error: Method `A#each` implements an abstract method `Enumerable#each` but is not declared with `override.`
  end
end
