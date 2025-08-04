# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::AppPackage::OtherClass # resolves via root
      Foo::Bar::AppPackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Test::Foo::Bar::AppPackage::TestUtil
end
