# frozen_string_literal: true
# typed: true
# compiled: true

arr = [1, 2, 3]
i = 0
while i < 1_000_000
  arr.all? do |x|
    x > 0
  end
  i += 1
end
puts i
