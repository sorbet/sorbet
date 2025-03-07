# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::AppFalseCyclePackage::OtherClass # resolves via root
      Bar::AppFalseCyclePackage::OtherClass # resolves via `module Foo`
    end
  end
end

module Foo::MyPackage
  Foo::Bar::AppFalseCyclePackage::OtherClass # resolves via root

  Test::Foo::Bar::AppFalseCyclePackage::TestUtil
end
