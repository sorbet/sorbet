# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::FalsePackage::OtherClass # resolves via root
      Bar::FalsePackage::OtherClass # resolves via `module Foo`
      Foo::Bar::FalseAndAppPackage::OtherClass # resolves via root
      Bar::FalseAndAppPackage::OtherClass # resolves via `module Foo`
    end
  end
end

module Foo::MyPackage
  Foo::Bar::FalsePackage::OtherClass # resolves via root

  Test::Foo::Bar::FalsePackage::TestUtil

  Foo::Bar::FalseAndAppPackage::OtherClass # resolves via root

  Test::Foo::Bar::FalseAndAppPackage::TestUtil
end
