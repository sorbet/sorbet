# typed: true

def foo # error: Hint: this "def" token is not closed before the end of the file
  for i in 1..7
end

def bar; end # error: unexpected token "end of file"
