# typed: true

module A # error: Hint: this "module" token is not closed before the end of the file
  module B
    module C
    end
  end # error: unexpected token "end of file"
