# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::FalseAndAppPackage::OtherClass # resolves via root
      Foo::Bar::FalseAndAppPackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Test::Foo::Bar::FalseAndAppPackage::TestUtil
end
