# frozen_string_literal: true
# typed: true
# compiled: true

result = [1, 2, 3]

result.map! do |x|
  puts "-- #{x} --"
  x + 1
end


p result

array = [1, 2, 3, 4, 5, 6, 7, 8]

result = array.map! do |x|
  break :finished if x == 6
  puts "-- #{x} --"
  x + 1
end

p result
p array
