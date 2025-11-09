# typed: true

module M
end

class A
  extend T::Generic
  Elem = type_member

  include M
end

class Test
  extend T::Sig

  sig {params(x: T.any(A[Integer], M)).void}
  def test_lub(x)
    T.reveal_type(x) # error: Revealed type: `M`
  end

  sig {params(x: T.all(A[Integer], M)).void}
  def test_glb(x)
    T.reveal_type(x) # error: Revealed type: `A[Integer]`
  end
end

module IBox
  extend T::Generic
  Elem = type_member
end

class MyGeneric
  extend T::Sig, T::Generic

  HasUpper = type_member { {upper: IBox[Integer]} }
  HasLower = type_member { {lower: IBox[Integer]} }

  sig { params(x: T.all(IBox[Integer], HasUpper)).void }
  def collapses_intersection_upper(x)
    T.reveal_type(x) # error: `MyGeneric::HasUpper`
  end

  sig { params(x: T.all(IBox[Integer], HasLower)).void }
  def collapses_intersection_lower(x)
    T.reveal_type(x) # error: `IBox[Integer]`
  end

  sig { params(x: T.any(IBox[Integer], HasUpper)).void }
  def collapses_union_upper(x)
    T.reveal_type(x) # error: `IBox[Integer]`
  end

  sig { params(x: T.any(IBox[Integer], HasLower)).void }
  def collapses_union_lower(x)
    T.reveal_type(x) # error: `MyGeneric::HasLower`
  end
end

class MyGenericUntyped
  extend T::Sig, T::Generic

  HasUpper = type_member { {upper: IBox[T.untyped]} }
  HasLower = type_member { {lower: IBox[T.untyped]} }

  sig { params(x: T.all(IBox[T.untyped], HasUpper)).void }
  def collapses_intersection_upper(x)
    T.reveal_type(x) # error: `T.all(IBox[T.untyped], MyGenericUntyped::HasUpper)`
  end

  sig { params(x: T.all(IBox[T.untyped], HasLower)).void }
  def collapses_intersection_lower(x)
    T.reveal_type(x) # error: `T.all(IBox[T.untyped], MyGenericUntyped::HasLower)`
  end

  sig { params(x: T.any(IBox[T.untyped], HasUpper)).void }
  def collapses_union_upper(x)
    T.reveal_type(x) # error: `IBox[T.untyped]`
  end

  sig { params(x: T.any(IBox[T.untyped], HasLower)).void }
  def collapses_union_lower(x)
    T.reveal_type(x) # error: `MyGenericUntyped::HasLower`
  end
end
