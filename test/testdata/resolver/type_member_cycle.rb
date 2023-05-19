# typed: true
# disable-fast-path: true

class A
  extend T::Sig
  extend T::Generic

  X = type_member {{fixed: B}}
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Type member `A::X` is involved in a cycle
end

class B
  extend T::Sig
  extend T::Generic

  X = type_member {{fixed: A}}
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Type member `B::X` is involved in a cycle

  sig {returns(X)}
  def test
    10
  end
end

T.reveal_type(B.new.test) # error: Revealed type: `T.untyped`

class C
  extend T::Generic

  Elem = type_member {{upper: self}} # error: Type member `C::Elem` is involved in a cycle
end
