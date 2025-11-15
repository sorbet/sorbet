# typed: strict

# type members redeclaration error disappears on fast path:
# https://github.com/sorbet/sorbet/issues/6699

# disable-fast-path: true

# This is a test of what happens if you use `T.self_type` in a type member.
#
# At time of writing, we don't explicitly ban this, but it introduces problems
# that make it effectively banned because it's unusable.
#
# One day we might want to allow this, and I just wanted to make sure that
# there were test cases reminding a potential future implementor to think about
# these cases. If this test case ever passes, it's possible that Sorbet is able
# to model something like a `Comparable` interface (see this issue:
# https://github.com/sorbet/sorbet/issues/6332)

class A
  extend T::Sig, T::Generic

  X = type_member { { fixed: T.self_type} }
  #                          ^^^^^^^^^^^ error: `T.self_type` is not supported inside generic type bounds

  sig { returns(X) }
  def my_dup = self
end

p(A)
a = A.new
T.reveal_type(a) # error: `A`

class ChildABad < A # error: Type `X` declared by parent `A` must be re-declared in `ChildABad`
end

class ChildA < A
  # should this be allowed?
  # it almost seems like you should be required to do `fixed: A`
  X = type_member { {fixed: T.self_type} }
  #                         ^^^^^^^^^^^ error: `T.self_type` is not supported inside generic type bounds
end

res = ChildA.new.my_dup
T.reveal_type(res) # error: `T.untyped`

class B
  extend T::Sig, T::Generic
  X = type_member { {lower: T.self_type, upper: T.self_type} }
  #                         ^^^^^^^^^^^ error: `T.self_type` is not supported inside generic type bounds
  #                                             ^^^^^^^^^^^ error: `T.self_type` is not supported inside generic type bounds

  sig { returns(X) }
  def my_dup = self
end

p(B)
b = B.new
T.reveal_type(b) # error: `B[T.untyped]`

class ChildB < B
  X = type_member { {lower: T.self_type, upper: T.self_type} }
  #                         ^^^^^^^^^^^ error: `T.self_type` is not supported inside generic type bounds
  #                                             ^^^^^^^^^^^ error: `T.self_type` is not supported inside generic type bounds
end

class C
  extend T::Sig, T::Generic
  X = type_member(:out) { {lower: T.self_type, upper: T.self_type} }
  #                               ^^^^^^^^^^^ error: `T.self_type` is not supported inside generic type bounds
  #                                                   ^^^^^^^^^^^ error: `T.self_type` is not supported inside generic type bounds

  sig { returns(X) }
  def my_dup = self
end

p(C)
b = C.new
T.reveal_type(b) # error: `C[T.untyped]`
