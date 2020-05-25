# typed: true
foo = a(b
if @c && foo # error: unexpected token "if"
  d # error: This code is unreachable
end

if @d && foo
end
