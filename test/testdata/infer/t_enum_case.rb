# typed: strict
extend T::Sig

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

sig {params(x: MyEnum).returns(Integer)}
def example(x)
  res = case x
  when MyEnum::X then 1
  when MyEnum::Y then 2
  end

  res # no error (cases are exhaustive so res is not nilable)
end
