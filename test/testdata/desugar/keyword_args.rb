# typed: true

def foo(x:, y: 10)
  p x
  p y
end

foo(x: 10)
foo(x: 10, y: 20)

args = {x: 10}
foo(**args)
foo(x: 20, **args)

other1= 10
foo(other1 => 10)
#   ^^^^^^^^^^^^ error: Cannot call `Object#foo` with a `Hash` keyword splat because the method has required keyword parameters
foo(**{other1 => 10})
#   ^^^^^^^^^^^^^^^^ error: Cannot call `Object#foo` with a `Hash` keyword splat because the method has required keyword parameters

other2 = "foo"
foo(other2 => 10)
#   ^^^^^^^^^^^^ error: Cannot call `Object#foo` with a `Hash` keyword splat because the method has required keyword parameters
foo(**{other2 => 10})
#   ^^^^^^^^^^^^^^^^ error: Cannot call `Object#foo` with a `Hash` keyword splat because the method has required keyword parameters

other3 = T.let(:foo, Symbol)
foo(other3 => 10)
#   ^^^^^^^^^^^^ error: Cannot call `Object#foo` with a `Hash` keyword splat because the method has required keyword parameters
foo(**{other3 => 10})
#   ^^^^^^^^^^^^^^^^ error: Cannot call `Object#foo` with a `Hash` keyword splat because the method has required keyword parameters
