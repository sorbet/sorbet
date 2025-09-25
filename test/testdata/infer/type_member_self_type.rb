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

  sig { returns(X) }
  def my_dup = self # error: Expression does not have a fully-defined type
end

p(A) # error: Expression does not have a fully-defined type
a = A.new # error: Expression does not have a fully-defined type
T.reveal_type(a) # error: `T.untyped`

class ChildABad < A # error: Type `X` declared by parent `A` must be re-declared in `ChildABad`
end

class ChildA < A
  # should this be allowed?
  # it almost seems like you should be required to do `fixed: A`
  X = type_member { {fixed: T.self_type} }
end

res = ChildA.new.my_dup
#     ^^^^^^ error: Expression does not have a fully-defined type
T.reveal_type(res) # error: `T.untyped`
