# typed: strict

module Consumer
  T.reveal_type(FromAssignment) # error: Revealed type: `Integer(1)`
end
