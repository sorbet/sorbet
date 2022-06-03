# typed: true

class MyEnum < T::Enum
  enums do
    Field1 = new
    Field2 = new
  end
end

MyEnum_Field1 = MyEnum::Field1

TestAlias = T.let(MyEnum::Field1, MyEnum)
