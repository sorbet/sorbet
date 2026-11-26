# typed: strict

module D
  CONSTANT_FROM_D = "Hello from Package D"

  class ClassFromD
    CONSTANT_FROM_D_CLASS = "Hello from ClassFromD"
  end

  FROM_B = T.let(B::CONSTANT_FROM_B, String)

  class EnumFromD < T::Enum
    enums do
      Variant = new
      OtherVariant = new
    end
  end

  class ShouldNotBeExported
  end
end
