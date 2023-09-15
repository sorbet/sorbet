# typed: true
extend T::Sig

class Box
  extend T::Sig
  extend T::Generic
  Elem = type_member(:out)

  sig {params(val: Elem).void}
  def initialize(val)
    @val = val
  end
end

class BoxA < Box
  Elem = type_member(:out) {{upper: A}}
end

sig {params(klass: T.class_of(Box)).void}
def example1(klass)
  T.reveal_type(klass) # error: `T.class_of(Box)[Box[T.anything]]`
  instance = klass.new(0)
  T.reveal_type(instance) # error: `Box[T.anything]`

  T.reveal_type(klass[Integer]) # error: `Runtime object representing type: Box[Integer]`
  instance = klass[Integer].new(0)
  T.reveal_type(instance) # error: `Box[Integer]`

  klass[Integer].new('')
  #                  ^^ error: Expected `Integer` but found `String("")` for argument `val`
end

module M; end
class A; end

class ChildA < A
  include M
end

class BoxChildA < Box
  Elem = type_member(:out) {{upper: ChildA}}
end

sig {params(klass: T.all(T.class_of(Box), T::Class[Box[T.all(A, M)]])).void}
def example2(klass)
  T.reveal_type(klass) # error: `T.class_of(Box)[Box[T.all(A, M)]]`
  instance = klass.new(ChildA.new)
  T.reveal_type(instance) # error: `Box[T.all(A, M)]`

  klass.new # error: Not enough arguments provided for method `Box#initialize`

  klass.new(A.new) # error: Expected `T.all(A, M)` but found `A` for argument `val`
  klass.new(0) # error: Expected `T.all(A, M)` but found `Integer(0)` for argument `val`

  # But I guess this still works, where you can just apply it to another type?

  T.reveal_type(klass[Integer]) # error: `Runtime object representing type: Box[Integer]`
  instance = klass[Integer].new # error: Not enough arguments
  T.reveal_type(instance) # error: `Box[Integer]`
end

example2(Box)
#        ^^^ error: Expected `T.class_of(Box)[Box[T.all(A, M)]]` but found `T.class_of(Box)`
example2(BoxA)
#        ^^^^ error: Expected `T.class_of(Box)[Box[T.all(A, M)]]` but found `T.class_of(BoxA)`
example2(BoxChildA)
