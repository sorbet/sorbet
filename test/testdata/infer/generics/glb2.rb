# @typed
class TestSubtyping
  class Parent
    extend T::Generic

    TParent = type_member
  end

  class SameArityChild < Parent
    TParent = type_member
  end

  class DifferentArityChild < Parent
      TParent = type_member
      TChild = type_member(fixed: String)
  end

  class TestGLB
    def test_it
      T.assert_type!(nil, T.all(SameArityChild[Integer], Parent[String])) # error: argument does not have asserted type T.all(SameArityChild[Integer], Parent[String])
      parent = T.cast(nil, Parent[Integer])
      if parent.is_a?(DifferentArityChild)
        T.assert_type!(parent, DifferentArityChild[Integer, String])
      end
    end
  end
end
