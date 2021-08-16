# typed: true
extend T::Sig

class A; end
class B; end
class C; end
class D < A; end


sig { returns(T.class_of(T.any(A, T.any(B, C)))) } # error: `T.class_of` must wrap each individual class type, not the outer `T.any`
def foo; end


sig { returns(T.class_of(T.any(A, D))) } # error: `T.class_of` needs a class or module as its argument
def bar; end
