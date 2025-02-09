# typed: strict
class BooleanTypeSuggestion
  extend T::Sig

  # Test typed boolean suggestion
  X = true # error: Constants must have type annotations
  Y = false # error: Constants must have type annotations

  # Test untyped boolean suggestion
  Z = T.unsafe(nil) # error: Constants must have type annotations
end
