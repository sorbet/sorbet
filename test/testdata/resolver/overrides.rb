# typed: true

# overriding a method not marked overridable without a sig should be okay
class A1
  def foo; end
end
class B1 < A1
  extend T::Sig
  sig { override.void }
  def foo; end
end

# overriding a method not marked overridable with a sig should be okay
class A2
  extend T::Sig
  sig { void }
  def foo; end
end
class B2 < A2
  extend T::Sig
  sig { override.void }
  def foo; end
end

# overriding an overridable method but not using a sig on the child should be okay
class A3
  extend T::Sig
  sig { overridable.void }
  def foo; end
end
class B3 < A3
  def foo; end
end

# overriding an overridable method with a non-override sig on the child should be an error
class A4
  extend T::Sig
  sig { overridable.void }
  def foo; end
end
class B4 < A4
  extend T::Sig
  sig { void }
  def foo; end # error: Method `B4#foo` overrides an overridable method `A4#foo` but is not declared with `.override`
end

# overriding an overridable method with an override sig on the child should be okay
class A5
  extend T::Sig
  sig { overridable.void }
  def foo; end
end
class B5 < A5
  extend T::Sig
  sig { override.void }
  def foo; end
end
