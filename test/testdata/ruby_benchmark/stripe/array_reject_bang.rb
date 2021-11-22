# frozen_string_literal: true
# typed: true
# compiled: true

arr = (1..10_000_000).to_a

arr.reject! do |x|
  x%2 == 0
end

p arr.length
