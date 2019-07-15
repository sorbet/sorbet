# typed: true

module Foo
  module Errors
    class BaseError < StandardError
      def foo
        'hi'
      end
    end

    class MyError1 < BaseError
      def foo
        '1'
      end
    end

    class MyError2 < BaseError
      def foo
        '2'
      end
    end
  end
end
