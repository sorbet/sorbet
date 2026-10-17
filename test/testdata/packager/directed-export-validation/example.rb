# typed: strict
module Example
  class Outer
    class Inner; end
  end
  class Enum < T::Enum
    enums do
      Value = new
    end
  end
  class Generic
    extend T::Generic
    Elem = type_member
  end
end
