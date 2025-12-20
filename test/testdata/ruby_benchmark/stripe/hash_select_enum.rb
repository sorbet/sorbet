# frozen_string_literal: true
# typed: true
# compiled: true

i = 0
z = 0

while i < 1_000_000
  xs = {x: 99, q: 48, j: 21, e: 18}
  e = xs.select
  ys = e.each { |k, v| k==:x || v%2==0 }
  z += ys.length

  i += 1
end

puts z
