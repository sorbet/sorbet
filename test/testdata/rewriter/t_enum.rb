# typed: strict

extend T::Sig

class MyEnum < T::Enum
  enums do
  X = new
  Y = new('y')
  Z = T.let(new, self)
  end
end

T.reveal_type(MyEnum::X) # error: Revealed type: `MyEnum::X`
T.reveal_type(MyEnum::Y) # error: Revealed type: `MyEnum::Y`
T.reveal_type(MyEnum::Z) # error: Revealed type: `MyEnum::Z`

class NotAnEnum
  enums do # error: does not exist
  X = new # error: Constants must have type annotations
  Y = T.let(new, self)
  end
end

T.reveal_type(NotAnEnum::X) # error: Revealed type: `T.untyped`
T.reveal_type(NotAnEnum::Y) # error: Revealed type: `NotAnEnum`

class EnumWithStrings < T::Enum
  enums do
    X = new
    Y = new("y")
    Z = new
  end
end

T.reveal_type(EnumWithStrings::X.serialize) # error: Revealed type: `String`

class EnumWithSymbols < T::Enum
  enums do
    X = new(:x)
    Y = new(:y)
    Z = new(:z)
  end
end

T.reveal_type(EnumWithSymbols::X.serialize) # error: Revealed type: `Symbol`

class EnumWithNonLiterals < T::Enum
  enums do
    X = new(:x.to_s)
    Y = new("y")
    Z = new(:z)
  end
end

T.reveal_type(EnumWithNonLiterals::X.serialize) # error: Revealed type: `T.untyped`

class EnumWithKwargs < T::Enum
  enums do
    X = new(a: 1, b: 2)
    Y = new(a: 3, b: 4)
  end
end

T.reveal_type(EnumWithKwargs::X.serialize) # error: Revealed type: `T.untyped`

class EnumWithNil < T::Enum
  enums do
    X = new(nil)
  end
end

T.reveal_type(EnumWithNil::X.serialize) # error: Revealed type: `NilClass`


class EnumWithPrivate < T::Enum
  extend T::Sig

  enums do
    X = new(nil)
  end

  private

  sig { void }
  def unrelated_method; end
end

EnumWithPrivate::X.serialize
