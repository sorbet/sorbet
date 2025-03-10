# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::FalsePackage::OtherClass # resolves via root
      Bar::FalsePackage::OtherClass # resolves via `module Foo`
      Foo::Bar::FalsePackageTest::OtherClass # resolves via root
    end
  end
end

module Foo::MyPackage
  Foo::Bar::FalsePackage::OtherClass # resolves via root

  Test::Foo::Bar::FalsePackage::TestUtil
end
