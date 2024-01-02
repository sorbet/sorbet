# typed: strict

class A; end
AliasToX = A

X = AliasToX.new # error: Constants must have type annotations with `T.let` when specifying `# typed: strict`
T.reveal_type(X) # error: `T.untyped`
