# typed: true

# highlight-untyped-values: true

extend T::Sig

sig { returns(Integer) }
def foo
  my_map = T.let({ foo: 1, bar: 'baz' }, T::Hash[Symbol, T.untyped])
  my_map[:foo]
# ^^^^^^^^^^^^ untyped: This code is untyped
end

sig { params(x: Integer, y: String).returns(Integer) }
def bar(x, y)
  if x > y.length
    x
  else
    y.length
  end
end

sig { returns(T.untyped) }
def baz
  T.let(5, T.untyped)
end

# assign untyped thing to variable
b = baz
#   ^^^ untyped: This code is untyped

# use an untyped variable
  b.length
# ^^^^^^^^ untyped: This code is untyped

  b.foo(0).bar(1).baz(2)
# ^^^^^^^^ untyped: This code is untyped
# ^^^^^^^^^^^^^^^ untyped: This code is untyped
# ^^^^^^^^^^^^^^^^^^^^^^ untyped: This code is untyped

T.let(b, Integer) == 6


my_map = T.let({:foo => 5, :bar => "foo"}, T::Hash[Symbol, T.untyped])
# untyped argument
bar(my_map[:foo], T.let("foo", T.untyped))
#   ^^^^^^^^^^^^ untyped: This code is untyped

# if condition
if my_map[:foo]
#  ^^^^^^^^^^^^ untyped: This code is untyped
  6
end

# case statement
case my_map[:bar]
#    ^^^^^^^^^^^^ untyped: This code is untyped
when "x"
  "x"
when "y"
  "y"
when b
#    ^ untyped: This code is untyped
  "b"
end

# use of super
class Base
  extend T::Sig

  sig { overridable.returns(String) }
  def foo
    "foo"
  end
end

class Derived < Base
  extend T::Sig
  sig { override.returns(String) }
  def foo
    super
#   ^^^^^ untyped: This code is untyped
  end
end

sig { params(x: Integer, y: String).returns(Integer) }
def binary_method(x, y)
  4
end

untyped_array_args = [T.let(1, T.untyped), T.let(1, T.untyped)]
# FIXME this should report an untyped error
binary_method(*untyped_array_args)

untyped_args = T.unsafe([])
#              ^^^^^^^^^^^^ untyped: This code is untyped

# FIXME the first assertion is also weird and needs
# to be fixed.
 binary_method(*untyped_args)
#     untyped: This code is untyped
#^^^^^^^^^^^^^^^^^^^^^^^^^^^^ untyped: This code is untyped

begin
# FIXME this should not be an untyped error
rescue StandardError
  #    ^^^^^^^^^^^^^ untyped: This code is untyped
end
