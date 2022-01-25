# typed: true
foo = a(b
if @c && foo # error: unexpected token "if"
  d
end # error: unexpected token "end"

if @d && foo
end
