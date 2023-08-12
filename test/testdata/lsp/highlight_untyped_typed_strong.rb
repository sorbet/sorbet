# typed: strong

# highlight-untyped-values: false

extend T::Sig

sig { returns(Integer) }
def foo
  my_map = T.let({ foo: 1, bar: 'baz' }, T::Hash[Symbol, T.untyped])
  my_map[:foo]
# ^^^^^^^^^^^^ error: Value returned from method is `T.untyped`
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
# ^^^^^^ error: Call to method `length` on `T.untyped`
T.let(b, Integer) == 6


my_map = T.let({:foo => 5, :bar => "foo"}, T::Hash[Symbol, T.untyped])
# untyped argument
bar(my_map[:foo], T.let("foo", T.untyped))
#   ^^^^^^^^^^^^ error: Argument passed to parameter `x` is `T.untyped`
#                 ^^^^^^^^^^^^^^^^^^^^^^^ error: Argument passed to parameter `y` is `T.untyped`

# if condition
if my_map[:foo]
#  ^^^^^^^^^^^^ error: Conditional branch on `T.untyped`
  puts(6)
end

# case statement
case my_map[:bar]
#    ^^^^^^^^^^^^ error: Argument passed to parameter `arg0` is `T.untyped`
#    ^^^^^^^^^^^^ error: Argument passed to parameter `arg0` is `T.untyped`
when "x"
  "x"
when "y"
  "y"
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
# ^^^^^^^^^ error: Call to method `call` on `T.untyped`
end
