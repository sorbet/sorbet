# typed: true

class A
  extend T::Sig
  extend T::Helpers
  abstract!

  sig { abstract.returns(T::Boolean).narrows_to(B) }
  def is_b?; end

  sig { abstract.returns(T::Boolean).narrows_to(C) }
  def is_c?; end

  sig { abstract.returns(T::Boolean).narrows_to(D) }
  def is_d?; end
end

class B < A
  sig { override.returns(TrueClass).narrows_to(B) }
  def is_b?; true; end

  sig { override.returns(FalseClass).narrows_to(C) }
  def is_c?; false; end

  sig { override.returns(FalseClass).narrows_to(D) }
  def is_d?; false; end
end

class C < A
  sig { override.returns(FalseClass).narrows_to(B) }
  def is_b?; false; end

  sig { override.returns(TrueClass).narrows_to(C) }
  def is_c?; true; end

  sig { override.returns(FalseClass).narrows_to(D) }
  def is_d?; false; end
end

class D < A
  abstract!
  sealed!

  sig { override.returns(FalseClass).narrows_to(B) }
  def is_b?; false; end

  sig { override.returns(FalseClass).narrows_to(C) }
  def is_c?; false; end

  sig { override.returns(TrueClass).narrows_to(D) }
  def is_d?; true; end

  sig { abstract.returns(T::Boolean).narrows_to(D1) }
  def is_d_1?; end

  sig { abstract.returns(T::Boolean).narrows_to(D2) }
  def is_d_2?; end
end

class D1 < D
  sig { override.returns(TrueClass).narrows_to(D1) }
  def is_d_1?; true; end

  sig { override.returns(FalseClass).narrows_to(D2) }
  def is_d_2?; false; end
end

class D2 < D
  sig { override.returns(FalseClass).narrows_to(D1) }
  def is_d_1?; false; end

  sig { override.returns(TrueClass).narrows_to(D2) }
  def is_d_2?; true; end
end

module Test
  extend T::Sig

  sig {params(a: T.any(B, C, D)).void}
  def test_union_three_way(a)
    if a.is_b?
      T.reveal_type(a) # error: Revealed type: `B`
    else
      T.reveal_type(a) # error: Revealed type: `T.any(C, D)`
      if a.is_d? && a.is_d_1?
        T.reveal_type(a) # error: Revealed type: `D1`
      else
        T.reveal_type(a) # error: Revealed type: `T.any(D2, C)`
      end
    end
  end
end
