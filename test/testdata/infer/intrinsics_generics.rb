# typed: true

Sorbet.sig {type_parameters(:A).params(a: T.type_parameter(:A)).void}
def foo(a)
 a.foo(1, *[2, 3])
end

