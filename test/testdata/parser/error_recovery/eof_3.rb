# typed: true

module A # parser-error: Hint: this "module" token is not closed before the end of the file
  module B # parser-error: Hint: this "module" token is not closed before the end of the file
    module # parser-error: Hint: this "module" token is not closed before the end of the file
      C # parser-error: unexpected token "end of file"
