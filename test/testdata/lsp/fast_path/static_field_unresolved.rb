# typed: true

class Outer
  module A; end

  A::B1 = T.let(0, Integer)

  A::B2 = T.let(0, Intege)
  #                ^^^^^^ error: Unable to resolve constant `Intege`
end

T.reveal_type(Outer::A::B1) # error: `Integer`

T.reveal_type(Outer::A::B2) # error: `Outer::Intege (unresolved)`
