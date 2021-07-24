# frozen_string_literal: true
# typed: true
# compiled: true

arr = [1, 2, 3]
i = 0
while i < 1_000_000
  arr.filter { |x| x.even? }
  i += 1
end
puts i
