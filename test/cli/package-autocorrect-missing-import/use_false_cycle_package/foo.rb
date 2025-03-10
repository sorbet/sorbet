# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::FalseCyclePackage::OtherClass # resolves via root
      Bar::FalseCyclePackage::OtherClass # resolves via `module Foo`
      Foo::Bar::FalseCyclePackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Foo::Bar::FalseCyclePackage::OtherClass # resolves via root

  Test::Foo::Bar::FalseCyclePackage::TestUtil
end
