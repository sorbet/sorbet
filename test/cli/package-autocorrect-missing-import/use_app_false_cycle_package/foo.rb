# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::AppFalseCyclePackage::OtherClass # resolves via root
      Foo::Bar::AppFalseCyclePackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Test::Foo::Bar::AppFalseCyclePackage::TestUtil
end
