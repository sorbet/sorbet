# typed: true

class A
  extend T::Sig
  extend T::Helpers

  abstract!

  sig { returns(T.untyped).narrows_to(B) }
  def is_b?; end
end

class B < A; end

module Test
  extend T::Sig

  sig { params(a: A).void }
  def test_untyped(a)
    if a.is_b?
      T.reveal_type(a) # error: Revealed type: `B`
    else
      T.reveal_type(a) # error: Revealed type: `A`
    end
  end
end
