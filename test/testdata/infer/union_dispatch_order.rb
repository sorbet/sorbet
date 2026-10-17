# typed: strict
extend T::Sig

class A
  extend T::Sig
  sig { returns(Integer) }
  def foo = 0
end

class B
end

class C
  extend T::Sig
  sig { returns(String) }
  def foo = ""
end

class D
end

class E
  extend T::Sig
  sig { returns(Symbol) }
  def foo = :e
end

sig { params(x: T.any(A, B, C, D, E)).void }
def left_nested(x)
  x.foo
  # ^^^ error: Method `foo` does not exist on `B` component of `T.any(A, B, C, D, E)`
  # ^^^ error: Method `foo` does not exist on `D` component of `T.any(A, B, C, D, E)`
end

sig { params(x: T.any(A, C, E)).void }
def return_type(x)
  T.reveal_type(x.foo) # error: Revealed type: `T.any(Integer, String, Symbol)`
end

sig { params(x: T.any(A, B), y: T.any(C, D, E), cond: T::Boolean).void }
def merged_unions(x, y, cond)
  z = cond ? y : x
  z.foo
  # ^^^ error: Method `foo` does not exist on `B` component of `T.any(A, B, C, D, E)`
  # ^^^ error: Method `foo` does not exist on `D` component of `T.any(A, B, C, D, E)`
end
