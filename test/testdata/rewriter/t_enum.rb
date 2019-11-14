# typed: strict

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
