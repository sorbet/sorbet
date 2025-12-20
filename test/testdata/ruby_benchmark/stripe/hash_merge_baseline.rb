# frozen_string_literal: true
# typed: true
# compiled: true

i = 0
z = 0

h1 = {x: 1, y: 2, z: 3, w: 4}
h2 = {x: 3, z: 9, j: 22}

while i < 10_000_000
  z += h1.length

  i += 1
end

puts z
