# typed: true
# TODO enable on the fast path

class DefaultBad < T::Struct
  prop :foo, Integer, default: "bad" # error: Returning value that does not conform to method result type
end
