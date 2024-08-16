# typed: true
extend T::Sig

class A; end
#     ^ type-def: A

sig { params(xyz: String).void }
def takes_string(xyz:)
end

  xyz = A
# ^^^ def: xyz
takes_string(xyz:)
#            ^^^^ error: Expected `String` but found `T.class_of(A)` for argument `xyz`
#            ^^^ hover: T.class_of(A)
#            ^^^ usage: xyz
#            ^^^ type: A
