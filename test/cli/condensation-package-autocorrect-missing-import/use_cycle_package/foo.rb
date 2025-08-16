# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::CyclePackage::OtherClass # resolves via root
      Foo::Bar::CyclePackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Test::Foo::Bar::CyclePackage::TestUtil
end
