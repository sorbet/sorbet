# typed: true

class A
  extend T::Sig
  extend T::Helpers
  abstract!

  sig { abstract.returns(T::Boolean).narrows_to(B) }
  def is_b?; end

  sig { abstract.returns(T::Boolean).narrows_to(C) }
  def is_c?; end
end

class B < A
  sig { override.returns(TrueClass).narrows_to(B) }
  def is_b?; true; end

  sig { override.returns(FalseClass) }
  def is_c?; false; end
end

class C < A
  sig { override.returns(FalseClass) }
  def is_b?; false; end

  sig { override.returns(TrueClass).narrows_to(C) }
  def is_c?; true; end
end

module Test
  extend T::Sig

  sig {params(a: A).void}
  def test_and_operator(a)
    if a.is_b? && a.is_c?
      T.reveal_type(a) # error: This code is unreachable
    end
  end
end
