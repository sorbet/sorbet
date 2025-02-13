# typed: true

class A # parser-error: Hint: this "class" token is not closed before the end of the file
  module B # parser-error: Hint: this "module" token is not closed before the end of the file
    def self.foo # parser-error: Hint: this "def" token is not closed before the end of the file
      def bar # parser-error: Hint: this "def" token is not closed before the end of the file

      puts('inside foo')
  # This is currently nested in the wrong scope.
  puts('inside A') # parser-error: unexpected token "end of file"
