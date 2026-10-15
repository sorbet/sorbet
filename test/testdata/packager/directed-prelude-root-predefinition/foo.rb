# typed: true

module MyPackage
  class Foo
    extend T::Generic

    Elem = type_member
  end
end
