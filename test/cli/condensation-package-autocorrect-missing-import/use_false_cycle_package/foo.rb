# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::FalseCyclePackage::OtherClass # resolves via root
      Foo::Bar::FalseCyclePackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Test::Foo::Bar::FalseCyclePackage::TestUtil
end
