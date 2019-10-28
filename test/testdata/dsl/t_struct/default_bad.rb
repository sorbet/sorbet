# typed: true
# TODO enable on the fast path

class DefaultBad < T::Struct
  prop :foo, Integer, default: "bad" # not-error Argument does not have asserted type `Integer`
end
