# frozen_string_literal: true
# typed: true
# compiled: true

result = [1, 2, 3, 4, 5, 6].all? do |x|
  puts x
  x.even?
end
puts result

result = [1, 2, 3, 4, 5, 6].all? do |x|
  puts x
  break :finished if x == 1
  x.even?
end
puts result

def foo(&blk)
  result = [1, 2, 3, 4, 5, 6].all?(&blk)
  puts result
end

foo do |x|
  puts x
  x.even?
end

result = [1, 2, 3, 4, 5, 6].all? do |x|
  puts x
  x > 0
end
puts result

arr = [1, 2, 3, 4, 5, 6]
result = arr.all? do |x|
  arr.pop
  puts x
  x > 0
end
puts result
