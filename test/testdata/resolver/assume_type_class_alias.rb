# typed: strict

class A; end
AliasToX = A

X = AliasToX.new
T.reveal_type(X) # error: `A`
