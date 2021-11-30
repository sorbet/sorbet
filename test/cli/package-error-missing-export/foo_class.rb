# typed: strict

module Foo
  module MyPackage::Buuzz
    class FooClass
      Foo::Bar::OtherPackage::OtherClass
      Bar::OtherPackage::OtherClass

      # Following are all not exported:
      Foo::Bar::OtherPackage::NotExported
      Bar::OtherPackage::NotExported
      Foo::Bar::OtherPackage::Inner::AlsoNotExported
      Bar::OtherPackage::Inner::AlsoNotExported
    end
  end
end
