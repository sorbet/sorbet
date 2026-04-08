# typed: true
extend T::Sig

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
    Z = new
  end
end

sig { params(my_enum: MyEnum).void }
def takes_my_enum(my_enum)
  takes_xyz(my_enum) # error: Expected `T.any(MyEnum::Z, MyEnum::X, MyEnum::Y)` but found `MyEnum` for argument `my_enum`
end
sig { params(my_enum: T.any(MyEnum::X, MyEnum::Y, MyEnum::Z)).void }
def takes_xyz(my_enum)
  takes_my_enum(my_enum)
end
