# frozen_string_literal: true
# typed: true
# compiled: true

hash = {x: {y: {z: "hi"}}}

i = 0
sum = 0

while i < 10_000_000
  i += 1

  sum += hash.dig(:x, :y, :z).length
end

puts i
puts sum
