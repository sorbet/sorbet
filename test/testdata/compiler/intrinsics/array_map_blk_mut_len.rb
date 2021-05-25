# frozen_string_literal: true
# typed: true
# compiled: true

xs = [1, 2, 3, 4]
result = xs.map do |x|
  xs.pop
  puts "-- #{x} --"
  x
end

p result
