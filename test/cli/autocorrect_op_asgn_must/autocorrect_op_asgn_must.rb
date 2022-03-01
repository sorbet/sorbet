# typed: true

extend T::Sig

class A
  extend T::Sig
  sig {returns(T.nilable(Integer))}
  attr_accessor :foo
end

sig do
  params(
    x: T.nilable(Integer),
    xs: T::Array[T.nilable(Integer)],
    opts: T::Hash[Symbol, T.nilable(Integer)],
    shape: {foo: T.nilable(Integer)},
    maybe_a: T.nilable(A),
  )
  .void
end
def example1(x, xs, opts, shape, maybe_a)
  # x += 1

  a = A.new
  a.foo += 1

  # This one is not great, because the autocorrect changes the meaning of the
  # program (doesn't increment the right instance of `A`).
  A.new.foo += 1

  xs[0] += 1

  opts[:foo] += 1

  # One day we will have better shape types, this will become an error, and
  # there should be an autocorrect.
  shape[:foo] += 1

  # This one is also not great, because the thing it suggests will error
  # because Sorbet will be able to know that the `&.` in T.must(maybe_a&.foo)
  # is redundant. We're fine with that being bad, because there's also an
  # auto-fix for that as well.
  maybe_a&.foo += 1

  # These aren't affected, because `||` and `&&` are available everywhere.
  bool = T.let(false, T.nilable(T::Boolean))
  bool ||= true
  bool = T.let(false, T.nilable(T::Boolean))
  bool &&= false
end
