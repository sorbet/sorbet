# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::AppCyclePackage::OtherClass # resolves via root
      Bar::AppCyclePackage::OtherClass # resolves via `module Foo`
      Foo::Bar::AppCyclePackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Foo::Bar::AppCyclePackage::OtherClass # resolves via root

  Test::Foo::Bar::AppCyclePackage::TestUtil
end
