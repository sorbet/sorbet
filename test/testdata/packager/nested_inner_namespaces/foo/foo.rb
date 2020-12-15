# typed: strict

module Foo
  Constant = "Foo"
  module Bar
    Constant = "Bar"
    module Baz
      Constant = "Baz"
    end
  end
end
