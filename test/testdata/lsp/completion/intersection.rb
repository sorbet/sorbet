# typed: true

module M
  def foo_common_1; end
  def foo_common_2; end
  def foo_only_on_a; end
end
module N
  def foo_common_1; end
  def foo_common_2; end
  def foo_only_on_b; end
end

module Nullary
  def some_method; end
end

module Unary
  def some_method(x); end
end

# The behavior here is the *same as* T.any. (Maybe you expected it to be the dual.)
# See the comment in union.rb for more.

def test
  ab = T.let(T.unsafe(nil), T.all(M, N))
  ab.foo_
#        ^ completion: foo_common_1, foo_common_2, foo_only_on_a, foo_only_on_b
# ^^^^^^^ error: does not exist on `M`
# ^^^^^^^ error: does not exist on `N`

  # TODO(jez) This is a weird case. There are two methods named `some_method`.
  # We've decided to show *all* methods, but the arity on each component is different.
  nullary_or_unary = T.let(T.unsafe(nil), T.all(Nullary, Unary))
  nullary_or_unary.some_
#                       ^ completion: some_method
# ^^^^^^^^^^^^^^^^ error: does not exist on `Nullary`
# ^^^^^^^^^^^^^^^^ error: does not exist on `Unary`
end
