# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::OtherPackage::OtherClass
    end
  end
end
