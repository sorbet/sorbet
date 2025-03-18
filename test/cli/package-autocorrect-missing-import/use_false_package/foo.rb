# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::FalsePackage::OtherClass # resolves via root
      Foo::Bar::FalsePackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Test::Foo::Bar::FalsePackage::TestUtil
end
