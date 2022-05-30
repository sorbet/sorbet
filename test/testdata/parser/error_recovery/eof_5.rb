# typed: true

class A
  module B
    def self.foo
      def bar

      puts('inside foo')
  # This is currently nested in the wrong scope.
  puts('inside A')
