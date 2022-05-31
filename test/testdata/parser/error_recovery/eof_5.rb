# typed: true

class A # error: Hint: this "class" token is not closed before the end of the file
  module B # error: Hint: this "module" token is not closed before the end of the file
    def self.foo # error: Hint: this "def" token is not closed before the end of the file
      def bar # error: Hint: this "def" token is not closed before the end of the file

      puts('inside foo')
  # This is currently nested in the wrong scope.
  puts('inside A') # error: unexpected token "end of file"
