# typed: false
extend T::Sig

module Example
  extend T::Sig
  extend T::Generic

  # Also need to delete the double space in other changes: `type_member  {{...}}`

  A1 = type_member(
    fixed: Integer # error: syntax for bounds has changed
  )
  A2 = type_member(
    lower: Integer, # error: syntax for bounds has changed
    upper: T.any(Integer, String)
  )

  B1 = type_member(
    :out,
    fixed: Integer # error: syntax for bounds has changed
  )
  B2 = type_member(
    :out,
    lower: Integer, # error: syntax for bounds has changed
    upper: T.any(Integer, String)
  )

  C1 = type_member(fixed: T.any( # error: syntax for bounds has changed
    Integer,
    String
  ))
  C2 = type_member(:out, fixed: T.any( # error: syntax for bounds has changed
    Integer,
    String
  ))
end
