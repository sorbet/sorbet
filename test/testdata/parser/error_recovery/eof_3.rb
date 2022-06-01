# typed: true

module A # error: Hint: this "module" token is not closed before the end of the file
  module B # error: Hint: this "module" token is not closed before the end of the file
    module # error: Hint: this "module" token is not closed before the end of the file
      C # error: unexpected token "end of file"
