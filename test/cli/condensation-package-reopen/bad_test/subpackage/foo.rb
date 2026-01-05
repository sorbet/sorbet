# typed: true

module Test::BadTest
  module Subpackage
    class Foo
      def method = puts "method!"
    end
  end
end
