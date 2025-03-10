# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::CyclePackage::OtherClass # resolves via root
      Bar::CyclePackage::OtherClass # resolves via `module Foo`
      Foo::Bar::CyclePackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Foo::Bar::CyclePackage::OtherClass # resolves via root

  Test::Foo::Bar::CyclePackage::TestUtil
end
