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

def test
  ab = T.let(T.unsafe(nil), T.all(M, N))
  ab.foo_
  #      ^ completion: foo_common_1, foo_common_2
  #  ^^^^ error: does not exist on `M`
  #  ^^^^ error: does not exist on `N`

  # This is a weird case. There are two methods named `some_method`.
  # It currently shows all methods with the same name, but the arity on each
  # component is different so it'll be impossible to call even though we
  # suggest it.
  nullary_or_unary = T.let(T.unsafe(nil), T.all(Nullary, Unary))
  nullary_or_unary.some_
  #                     ^ completion: some_method
  #                ^^^^^ error: does not exist on `Nullary`
  #                ^^^^^ error: does not exist on `Unary`
end
