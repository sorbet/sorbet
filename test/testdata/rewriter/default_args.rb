# typed: true

extend T::Sig
sig {params(a: String, b: Integer, c: Integer).void}
def foo(a, b=1, c=2)
end
