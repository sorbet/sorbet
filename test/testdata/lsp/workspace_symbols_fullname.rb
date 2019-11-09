# typed: true

module A; module B; module C; end; end; end  

module A::B::C
  module FooBarBaz1
    #    ^^^^^^^^^^ symbol-search: "fbb"
    def self.d; end
    class FooBarBaz2
      #   ^^^^^^^^^^ symbol-search: "fbb"
      def self.d; end
      def e; end
    end
  end

  class FooBarBaz3
    #   ^^^^^^^^^^ symbol-search: "fbb"
    def self.d; end
    def e; end
  end

  module Foo
    module Bar
      class Baz
        #   ^^^ symbol-search: "fbb"
        def self.d; end
        def e; end
      end
    end
    class BarBaz
      #   ^^^^^^ symbol-search: "fbb"
      def self.d; end
      def e; end
    end
  end
end
