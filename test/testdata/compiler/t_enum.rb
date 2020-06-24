# frozen_string_literal: true
# typed: strict
# compiled: true

class MyEnum < T::Enum
  enums do
    X = new
    Y = new('y')
    Z = T.let(new, self)
  end
end

p MyEnum::X
p MyEnum::Y
p MyEnum::Z
