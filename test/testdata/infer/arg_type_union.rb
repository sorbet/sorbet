# typed: true
extend T::Sig

class A
  extend T::Sig
  sig { params(x: String).void }
  def foo(x)
    puts x
  end
end

class B
  extend T::Sig
  sig { params(x: Symbol).void }
  def foo(x)
    puts x
  end
end

sig { returns(T.any(A, B)) }
def target
  [A.new, B.new].sample
end

target.foo(:symbol) # error: Expected `String` but found `Symbol(:symbol)` for argument `x` on `A#foo` component of `T.any(A, B)`
target.foo("string") # error: Expected `Symbol` but found `String("string")` for argument `x` on `B#foo` component of `T.any(A, B)`
