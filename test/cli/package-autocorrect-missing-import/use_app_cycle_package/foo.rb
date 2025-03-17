# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::AppCyclePackage::OtherClass # resolves via root
      Foo::Bar::AppCyclePackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Test::Foo::Bar::AppCyclePackage::TestUtil
end
