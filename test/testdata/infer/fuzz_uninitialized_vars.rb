# typed: true
foo = a(b
if @c && foo # error: unexpected token kIF
  d # error: This code is unreachable
end

if @d && foo
end
