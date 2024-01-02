# typed: strict

class A; end

AliasToA = A
X = AliasToA.new
T.reveal_type(X) # error: `A`

TypeAliasToA = T.type_alias { A }

# There should also be a type error on the call to `.new`, but that's also broken.
# See test/testdata/infer/metatype_new.rb for more.
Y = TypeAliasToA.new # error: Constants must have type annotations with `T.let` when specifying `# typed: strict`
T.reveal_type(Y) # error: `T.untyped`

