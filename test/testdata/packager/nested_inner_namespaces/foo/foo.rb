# typed: strict

module RootPackage::Foo
  Constant = "Foo"
  module Bar
    Constant = "Bar"
    module Baz
      Constant = "Baz"
    end
  end
end
