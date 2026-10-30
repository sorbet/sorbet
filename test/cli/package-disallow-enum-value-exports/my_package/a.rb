# typed: strict

module MyPackage
  class A < T::Enum
    enums do
      Val1 = new
      Val2 = new
    end

    # Not enum values, even though they're constants in an enum class
    NOT_A_VALUE = 1
    OTHER_CONST = 2
  end
end
