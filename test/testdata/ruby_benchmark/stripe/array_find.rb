# frozen_string_literal: true
# typed: true
# compiled: true

arr = [1, 2, 3]
i = 0
while i < 1_000_000
  arr.find { |x| x == 3 }
  i += 1
end
puts i
