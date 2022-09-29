# typed: true
# spacer for exclude-from-file-update

T.reveal_type( # error: `Integer`
  A::X
)
T.reveal_type( # error: `T.untyped`
  A::Y # error: Unable to resolve constant `Y`
)
