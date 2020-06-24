# frozen_string_literal: true
# typed: strict
# compiled: true

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

class Main
  extend T::Sig

  sig {params(my_enum: MyEnum).void}
  def self.main(my_enum)
    case my_enum
    when MyEnum::X
    when MyEnum::Y
    else
      T.absurd(my_enum)
    end
  end
end
