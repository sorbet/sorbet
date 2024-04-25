# frozen_string_literal: true
# typed: true
# compiled: true

xs = [1, 2, 3]
result = xs.each do |x|
  puts "-- #{x} --"
  xs.clear
end

p result
