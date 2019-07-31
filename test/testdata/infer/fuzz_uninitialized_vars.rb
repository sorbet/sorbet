# typed: true
foo = a(b
if @c && foo # error: Parse Error: unexpected token: syntax error
  d # error: This code is unreachable
end

if @d && foo
end
