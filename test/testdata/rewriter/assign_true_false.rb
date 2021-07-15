# typed: strict

extend T::Sig

sig {void}
def example1
  x = false

  T.reveal_type(x) # error: `T::Boolean`
  1.times do
    x = true
    T.reveal_type(x) # error: `T::Boolean`
  end

  T.reveal_type(x) # error: `T::Boolean`
end


class A
  extend T::Sig

  # Does not require `T.let` annotation, even in `typed: strict`
  X = true
  T.reveal_type(X) # error: `T::Boolean`

  sig {void}
  def foo
    @x = true
  # ^^ error: Use of undeclared variable `@x`
    T.reveal_type(@x) # error: `T::Boolean`
  end

  sig {void}
  def bar
    x = @x # error: Use of undeclared variable `@x`
    T.reveal_type(x) # error: `T.untyped`
  end
end

sig {params(x: TrueClass).void}
def takes_true(x); end

sig {void}
def example2
  x = true
  takes_true(x) # error: Expected `TrueClass` but found `T::Boolean` for argument `x`

  y = T.let(true, TrueClass)
  takes_true(y)
end
