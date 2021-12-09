# typed: strict

module Opus
  module Foo
    module Bar
      class BarClass
        Opus::Foo::FooClass.fooclass
        FooClass.fooclass

        AAA::AClass
        BBB::BClass
      end
    end
  end
end
