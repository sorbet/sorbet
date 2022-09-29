# typed: true

X = Integer

T.reveal_type( # error: `T.class_of(Integer)`
  X
)
T.reveal_type( # error: `T.untyped`
  Y # error: Unable to resolve constant `Y`
)
