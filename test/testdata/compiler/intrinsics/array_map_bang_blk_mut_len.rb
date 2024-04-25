# frozen_string_literal: true
# typed: true
# compiled: true

xs = T.let([1, 2, 3, 4], T::Array[Integer])
xs.map! do |x|
  xs.pop
  puts "-- #{x} --"
  x
end

p xs
