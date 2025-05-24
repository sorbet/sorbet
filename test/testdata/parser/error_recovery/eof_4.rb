# typed: true

module A # parser-error: Hint: this "module" token is not closed before the end of the file
  module B
    module C
    end
  end # parser-error: unexpected token "end of file"
