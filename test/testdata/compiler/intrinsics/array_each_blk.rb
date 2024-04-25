# frozen_string_literal: true
# typed: true
# compiled: true

result = [1, 2, 3].each do |x|
  puts "-- #{x} --"
end

p result

result = [1, 2, 3, 4, 5, 6, 7, 8].each do |x|
  puts "-- #{x} --"
  break :finished if x == 5
end

p result
