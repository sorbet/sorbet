# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::FalseAndAppPackage::OtherClass # resolves via root
      Bar::FalseAndAppPackage::OtherClass # resolves via `module Foo`
      Foo::Bar::FalseAndAppPackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Foo::Bar::FalseAndAppPackage::OtherClass # resolves via root

  Test::Foo::Bar::FalseAndAppPackage::TestUtil
end
