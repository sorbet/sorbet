# typed: strict

A = String.new
B = T.let(T.unsafe(nil), T.untyped)

C = T.let(1, Integer)

D = T.type_alias {Integer}
E = '' + '' # error: Constants must have type annotations with `T.let` when specifying `# typed: strict`
