# typed: true

class A < T::Struct
  prop :foo, T.nilable(Integer), default: '' # error-with-dupes: Argument does not have asserted type `T.nilable(Integer)`
end
