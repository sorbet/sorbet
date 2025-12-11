# typed: strict

module D
  CONSTANT_FROM_D = "Hello from Package D"

  class ClassFromD
    CONSTANT_FROM_D_CLASS = "Hello from ClassFromD"
  end

  class EnumFromD < T::Enum
    enums do
      Variant = new
      OtherVariant = new
    end
  end
end
