# typed: true
extend T::Sig

class A; end
class B; end
class C; end
class D < A; end

sig { returns(T.class_of(T.any(A, T.any(B, C)))) }
def foo; end

sig { returns(T.class_of(T.any(A, D))) }
def bar; end
