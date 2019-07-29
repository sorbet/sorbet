# typed: true

class ParamOrder < T::Struct
  prop :foo, Integer, default: 3
  prop :bar, Integer
end
