# frozen_string_literal: true
# typed: true
# compiled: true

result = [1, 2, 3, 4, 5, 6].filter do |x|
  x.even?
end

puts result

result = [1, 2, 3, 4, 5, 6].select do |x|
  x.even?
end

puts result

def foo(&blk)
  result = [1, 2, 3, 4, 5, 6].filter(&blk)
  puts result
end

foo do |x|
  x.even?
end
