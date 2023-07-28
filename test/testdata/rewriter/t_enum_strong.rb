# typed: strong

extend T::Sig

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end
