# typed: true
extend T::Sig

class A; end
class B; end
class C; end
class D < A; end


sig { returns(T.class_of(T.any(A, T.any(B, C)))) } # error: `T.class_of` needs a class or module as its argument. Apply `T.class_of` to each element of the `T.any` instead
def foo; end


sig { returns(T.class_of(T.any(A, D))) } # error: `T.class_of` needs a class or module as its argument
def bar; end
