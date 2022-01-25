# typed: true

class A
  def self.hello; end
end

class B < A; end
class C < A; end

class E; end

T.reveal_type(A.subclasses) # error: Revealed type: `T::Array[T.class_of(A)]`
T.reveal_type(B.subclasses) # error: Revealed type: `T::Array[T.class_of(B)]`
T.reveal_type(C.subclasses) # error: Revealed type: `T::Array[T.class_of(C)]`
T.reveal_type(E.subclasses) # error: Revealed type: `T::Array[T.class_of(E)]`

A.subclasses.each(&:hello)
E.subclasses.each(&:hello) # error: Method `hello` does not exist on `T.class_of(E)`
