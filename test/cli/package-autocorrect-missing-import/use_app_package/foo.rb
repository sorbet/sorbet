# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::AppPackage::OtherClass # resolves via root
      Bar::AppPackage::OtherClass # resolves via `module Foo`
    end
  end
end

module Foo::MyPackage
  Foo::Bar::AppPackage::OtherClass # resolves via root

  Test::Foo::Bar::AppPackage::TestUtil
end
