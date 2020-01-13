# typed: true

require 'minitest/autorun'
require 'minitest/spec'

module Scope
  class CustomThingTest
    describe 'foo.bar() with no args' do
      it 'will baz the qux' do
        # ^^^^^^^ symbol-search: "CustomThing describe bar it baz"
        # ^^^^^^^ symbol-search: "f b b q"
      end
    end
  end

  module FooBar
    def baz_qux
      # ^^^^^^^ symbol-search: "f b b q"
    end
    class Baz
      def qux
        # ^^^ symbol-search: "f b b q"
      end
    end
  end

  module Fob
    def baz_qux
      # // Shouldn't match because b in "Fob" isn't word boundary
    end
    class Arby
      def baz_qux
        # // Also shouldn't match because b in "Arby" isn't word boundary
      end
    end
    class AndBarAndBaz
      def qux
        # ^^^ symbol-search: "f b b q"
      end
    end
  end
end