# typed: true

module Foo
  module Bar
    class A
      def bar
      end
    end
  end

  class Bar::B
    def baz
    end
  end
end
