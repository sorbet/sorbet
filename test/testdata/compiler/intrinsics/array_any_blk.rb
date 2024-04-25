# frozen_string_literal: true
# typed: true
# compiled: true

result = [1, 2, 3, 4, 5, 6].any? do |x|
  puts x
  x.even?
end
puts result

result = [1, 2, 3, 4, 5, 6].any? do |x|
  puts x
  break :finished if x == 1
  x.even?
end
puts result

def foo(&blk)
  result = [1, 2, 3, 4, 5, 6].any?(&blk)
  puts result
end

foo do |x|
  puts x
  x.even?
end

result = [1, 2, 3, 4, 5, 6].any? do |x|
  puts x
  x > 6
end
puts result

arr = [1, 2, 3, 4, 5, 6]
result = arr.any? do |x|
  arr.pop
  puts x
  x > 6
end
puts result
