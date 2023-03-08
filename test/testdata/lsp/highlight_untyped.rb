# typed: true

# highlight-untyped-values: true

extend T::Sig

sig { returns(Integer) }
def foo
  my_map = T.let({ foo: 1, bar: 'baz' }, T::Hash[Symbol, T.untyped])
  my_map[:foo]
# ^^^^^^^^^^^^ info: This code is untyped
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
#   ^^^ info: This code is untyped

# use an untyped variable
  b.length
# ^^^^^^^^ info: This code is untyped
T.let(b, Integer) == 6


my_map = T.let({:foo => 5, :bar => "foo"}, T::Hash[Symbol, T.untyped])
# untyped argument
bar(my_map[:foo], T.let("foo", T.untyped))
#   ^^^^^^^^^^^^ info: This code is untyped

# if condition
if my_map[:foo]
#  ^^^^^^^^^^^^ info: This code is untyped
	6
end

# case statement
case my_map[:bar]
#    ^^^^^^^^^^^^ info: This code is untyped
when "x"
	"x"
when "y"
	"y"
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
# ^^^^^ info: This code is untyped
	end
end

# untyped varargs
sig { params(args: T.untyped).void }
def args_fn(*args)
	args.length
end

# untyped kwargs
sig { params(kwargs: T.untyped).void }
def kwargs_fn(**kwargs)
	kwargs.keys
end

# use of &blk with untyped blk
sig {params(blk: T.untyped).returns(T.untyped)}
def blk_fun(&blk)
	yield "x"
#^^^^^^^^^ info: This code is untyped
end 