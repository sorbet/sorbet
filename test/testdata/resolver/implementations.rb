# typed: true

# implementing an abstract method but not using a sig on the child is temporarily okay
class A1
  extend T::Sig
  extend T::Helpers
  abstract!
  sig { abstract.void }
  def foo; end
end
class B1 < A1
  def foo; end
end

# implementing an abstract method without implementation sig on the child should be an error
class A2
  extend T::Sig
  extend T::Helpers
  abstract!
  sig { abstract.void }
  def foo; end
end
class B2 < A2
  extend T::Sig
  sig { void }
  def foo; end # error: Method `B2#foo` implements an abstract method `A2#foo` but is not declared with `override.`
end

# implementing an abstract method with an implementation sig on the child should be okay
class A3
  extend T::Sig
  extend T::Helpers
  abstract!
  sig { abstract.void }
  def foo; end
end
class B3 < A3
  extend T::Sig
  sig { override.void }
  def foo; end
end
