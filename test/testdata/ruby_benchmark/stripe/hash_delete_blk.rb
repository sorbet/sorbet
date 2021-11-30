# frozen_string_literal: true
# typed: true
# compiled: true

h = {}

i = 0
while i < 10_000_000
  h[i] = i
  i += 2
end

i = 0
z = 0
while i < 10_000_000
  z += h.delete(i) {0}

  i += 1
end

puts i
puts z
