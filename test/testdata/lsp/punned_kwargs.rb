# typed: true
extend T::Sig

class A; end
#     ^ type-def: A

sig { params(xyz: String).void }
#            ^ def: xyz 1 not-def-of-self
def takes_string(xyz:)
end

  xyz = A

takes_string(xyz:)
#            ^^^^ error: Expected `String` but found `T.class_of(A)` for argument `xyz`
#            ^ hover-line: 2 # Object#takes_string
#            ^ hover-line: 3 (kwparam) xyz: String
#            ^ go-to-def-special: xyz
