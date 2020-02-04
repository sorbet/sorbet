# frozen_string_literal: true
# typed: true
# compiled: true

[1, 2, 3].each do |x, y|
  puts "-- x: #{x}, y: #{y.inspect} --"
end

[1, 2, 3].each do
  puts "-- hello --"
end
