# frozen_string_literal: true
# typed: true
# compiled: true

def foo7_blk(a, b, c, d, e, f, g, &blk)
end

i = 0
while i < 10_000_000

  foo7_blk(1, 2, 3, 4, 5, 6, 7) do end

  i += 1
end

puts i
