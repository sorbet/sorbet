# typed: strict

# warn-untyped-values: true

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

b = baz
#   ^^^ info: This code is untyped
  b.length
# ^^^^^^^^ info: This code is untyped
T.let(b, Integer) == 6