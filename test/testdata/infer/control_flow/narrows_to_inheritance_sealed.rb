# typed: true

class Parent
  extend T::Sig
  extend T::Helpers

  abstract!
  sealed!

  sig { returns(T::Boolean).narrows_to(Child1) }
  def is_child_1?; false; end
end

class Child1 < Parent
  sig { override.returns(TrueClass).narrows_to(Child1) }
  def is_child_1?; true; end
end

class Child2 < Parent; end

class Child3 < Parent; end

module Test
  extend T::Sig

  sig {params(parent: Parent).void}
  def test_inheritance(parent)
    if parent.is_child_1?
      T.reveal_type(parent) # error: Revealed type: `Child1`
    else
      T.reveal_type(parent) # error: Revealed type: `T.any(Child2, Child3)`
    end
  end
end
