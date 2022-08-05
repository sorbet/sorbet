# typed: true
extend T::Sig
sig {type_parameters(:A).params(a: T.type_parameter(:A)).void}
def foo(a)
 a.foo(1, *[2, 3])
 # ^^^ error: Call to method `foo` on unconstrained generic type `T.type_parameter(:A) (of Object#foo)`
end

