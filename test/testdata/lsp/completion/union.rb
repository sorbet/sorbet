# typed: true

class A
  def foo_common_1; end
  def foo_common_2; end
  def foo_only_on_a; end
end
class B
  def foo_common_1; end
  def foo_common_2; end
  def foo_only_on_b; end
end

class Nullary
  def some_method; end
end

class Unary
  def some_method(x); end
end

# Note how we suggest methods on either component. This is intential. People
# would think it's a bug if completion "stopped working" for nilable receivers.
# We'd rather show them the method then show them that it doesn't type check
# because it's nil.

def test
  ab = T.let(A.new, T.any(A, B))
  ab.foo_
#        ^ completion: foo_common_1, foo_common_2, foo_only_on_a, foo_only_on_b
# ^^^^^^^ error: Method `foo_` does not exist on `A` component of `T.any(A, B)`
# ^^^^^^^ error: Method `foo_` does not exist on `B` component of `T.any(A, B)`

  maybe_a = T.let(nil, T.nilable(A))
  maybe_a.foo_only
#                 ^ completion: foo_only_on_a
# ^^^^^^^^^^^^^^^^ error: Method `foo_only` does not exist on `A` component of `T.nilable(A)`
# ^^^^^^^^^^^^^^^^ error: Method `foo_only` does not exist on `NilClass` component of `T.nilable(A)`

  # TODO(jez) This is a weird case. There are two methods named `some_method`.
  # We've decided to show *all* methods, not just the intersection of methods.
  # And, the arity on each component is different. But we'll only show one of
  # the sigs, arbitrarily.
  nullary_or_unary = T.let(Nullary.new, T.any(Nullary, Unary))
  nullary_or_unary.some_
#                       ^ completion: some_method
# ^^^^^^^^^^^^^^^^^^^^^^ error: does not exist on `Unary`
# ^^^^^^^^^^^^^^^^^^^^^^ error: does not exist on `Nullary`
end
