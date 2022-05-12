# typed: __STDLIB_INTERNAL

class A
  class <<self
    extend T::Generic
    X = type_member(:out) {{upper: String}}

    class <<self
      extend T::Generic
      X = type_member(:out) {{upper: Integer}}
    end
  end
end
