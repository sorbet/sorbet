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
  def test_method
    10
  end
end

# This error is not great. This happens because the type member cycle on `B`
# prevents us from resolving `<AttachedClass>` on `T.class_of(B)`. We should
# probably recover from this problem better, making it so that
# `<AttachedClass>` is not bounded by `<todo sym>` even when this error
# happens.
#
# Before, we got around this because both `Foo.new` and `self.new` had
# intrinsics that forcibly set the result type based on the type of the
# receiver, but there was always this lurking problem.
b = B.new
T.reveal_type(b) # error: `<todo sym>`
res = B.new.test_method # error: Method `test_method` does not exist on `<todo sym>`
T.reveal_type(res) # error: `T.untyped`

class C
  extend T::Generic

  Elem = type_member {{upper: self}} # error: Type member `C::Elem` is involved in a cycle
end
