# typed: true

# // test for names that can be defined multiple times

module Foo
  module Bar
    #    ^^^ symbol-search: "Foo::Bar"
    class Baz
      #   ^^^ symbol-search: "Foo::Bar::Baz"
    end
    QUX = 3
  # ^^^ symbol-search: "Foo::Bar.QUX"
  end
end
