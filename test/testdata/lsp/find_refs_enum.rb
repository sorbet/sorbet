# typed: true

class MyEnum < T::Enum
  enums do
    X = new
  # ^ def: X
  end
end

puts(MyEnum::X)
#            ^ usage: X
