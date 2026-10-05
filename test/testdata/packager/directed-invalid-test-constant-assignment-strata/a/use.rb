# typed: strict

T.reveal_type(Test::Root::A::InvalidConstant) # error: Revealed type: `Integer(1)`
