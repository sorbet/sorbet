# typed: strict
class BooleanTypeSuggestion
  extend T::Sig
  extend T::Generic

  X = true # error: Constants must have type annotations
  Y = false # error: Constants must have type annotations

  BoundedType = type_member(:out) { {upper: FalseClass} }
  Z = T.let(nil, BoundedType) # error: Constants must have type annotations

  W = T.unsafe(nil) # error: Constants must have type annotations
end
