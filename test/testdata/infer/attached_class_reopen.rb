# typed: true

# This exposes a potential problem with the use of `externalType` in defining
# the upper bound of `AttachedClass`. When `AttachedClass` is updated to have
# the upper bound of `externalType` in `ResolveTypeParamsWalk`, it will run
# after each class definition. Running at the end of the first definition of `A`
# means that it misses that `Y` is fixed as `String`, and the type will end up
# as:
#
# > A[Integer,T.untyped]
#
# Instead, the upper bound of `AttachedClass` is computed in
# `ResolveSignaturesWalk`, which runs after all type members have been resolved
# fully producing this type instead:
#
# > A[Integer,String]

class A
  extend T::Sig
  extend T::Generic

  X = type_member {{fixed: Integer}}

  sig {returns(X)}
  def test_X
    10
  end
end

# Not interesting, but splits up the statements used to define A
class B; end

class A
  Y = type_member {{fixed: String}}

  sig {returns(Y)}
  def test_Y
    "foo"
  end
end

T.reveal_type(A.new) # error: Revealed type: `A`
T.reveal_type(A.new.test_X) # error: Revealed type: `Integer`
T.reveal_type(A.new.test_Y) # error: Revealed type: `String`
