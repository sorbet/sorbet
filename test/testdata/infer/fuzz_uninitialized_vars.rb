# typed: true
foo = a(b
if @c && foo # parser-error: unexpected token "if"
  d
end

if @d && foo
end
