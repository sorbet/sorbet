# typed: strict
# TODO enable on the fast path

class DefaultBad < T::Struct
  prop :foo, Integer, default: "bad" # error-with-dupes: Argument does not have asserted type `Integer`
end
