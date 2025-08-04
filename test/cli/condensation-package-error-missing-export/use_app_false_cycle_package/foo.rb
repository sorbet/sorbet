# typed: strict

module Foo
  module MyPackage
    class FooClass
      Foo::Bar::AppFalseCyclePackage::OtherClass
    end
  end
end
