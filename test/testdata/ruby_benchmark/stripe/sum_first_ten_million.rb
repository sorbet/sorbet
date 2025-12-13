# frozen_string_literal: true
# typed: true
# compiled: true

xs = Array.new(10_000_000) {|i| i}
sum = 0
xs.each do |i|
  sum += i
end
puts sum
