# frozen_string_literal: true
# typed: true
# compiled: true

def foo7(a, b, c, d, e, f, g)
end

i = 0
while i < 10_000_000

  foo7(1, 2, 3, 4, 5, 6, 7)

  i += 1
end

puts i
