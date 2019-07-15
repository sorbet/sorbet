# typed: true
extend T::Sig
class C; end
sig { params(x: T.all(T::Enumerable[Integer], C)).void }
def foo(x); x.each { |i| puts i }; end
