# typed: true

def foo
  for i in 1..7 # error: Hint: this "for" token might not be properly closed
end

def bar; end # error: unexpected token "end of file"
