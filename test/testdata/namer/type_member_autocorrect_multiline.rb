# typed: true
extend T::Sig

class Example
  extend T::Sig
  extend T::Generic

  # Also need to delete the double space in other changes: `type_member  {{...}}`

  A1 = type_member(
    fixed: Integer
  )
  A2 = type_member(
    lower: Integer,
    upper: T.any(Integer, String)
  )

  B1 = type_member(
    :out,
    fixed: Integer
  )
  B2 = type_member(
    :out,
    lower: Integer,
    upper: T.any(Integer, String)
  )

  C1 = type_member(fixed: T.any(
    Integer,
    String
  ))
  C2 = type_member(:out, fixed: T.any(
    Integer,
    String
  ))
end
