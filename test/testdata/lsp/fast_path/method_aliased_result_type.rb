# typed: true
extend T::Sig

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

MyEnum_X = MyEnum::X

sig {returns(MyEnum)}
def example
  MyEnum::X
end
