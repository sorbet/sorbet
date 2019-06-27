# typed: true

class DefaultBad < T::Struct
  prop :foo, Integer, default: "bad" # error: Argument does not have asserted type `Integer`
end
