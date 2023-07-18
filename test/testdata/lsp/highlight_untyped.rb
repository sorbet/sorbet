# typed: true

# highlight-untyped-values: true

extend T::Sig

sig { returns(Integer) }
def foo
  my_map = T.let({ foo: 1, bar: 'baz' }, T::Hash[Symbol, T.untyped])
  my_map[:foo]
# ^^^^^^^^^^^^ untyped: Value returned from method is `T.untyped`
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

# use an untyped variable
b.length
# ^^^^^^ untyped: Call to method `length` on `T.untyped`

b.foo(0).bar(1).baz(2)
# ^^^ untyped: Call to method `foo` on `T.untyped`
#        ^^^ untyped: Call to method `bar` on `T.untyped`
#               ^^^ untyped: Call to method `baz` on `T.untyped`

T.let(b, Integer) == 6


my_map = T.let({:foo => 5, :bar => "foo"}, T::Hash[Symbol, T.untyped])
# untyped argument
bar(my_map[:foo], T.let("foo", T.untyped))
#   ^^^^^^^^^^^^ untyped: Argument passed to parameter `x` is `T.untyped`
#                 ^^^^^^^^^^^^^^^^^^^^^^^ untyped: Argument passed to parameter `y` is `T.untyped`

# if condition
if my_map[:foo]
  #^^^^^^^^^^^^ untyped: Conditional branch on `T.untyped`
  puts(6)
end

# case statement
case my_map[:bar]
#    ^^^^^^^^^^^^ untyped: Argument passed to parameter `arg0` is `T.untyped`
#    ^^^^^^^^^^^^ untyped: Argument passed to parameter `arg0` is `T.untyped`
when "x"
  "x"
when "y"
  "y"
when b
#    ^ untyped: Call to method `===` on `T.untyped`
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
#   ^^^^^ untyped: Value returned from method is `T.untyped`
  end
end

sig { params(x: Integer, y: String).returns(Integer) }
def binary_method(x, y)
  4
end
