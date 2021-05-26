# frozen_string_literal: true
# typed: true
# compiled: true

result = [1, 2, 3, 4, 5, 6].find do |x|
  puts x
  x.even?
end

puts result

result = [1, 2, 3, 4, 5, 6].find(-> {puts "hello"; 10}) do |x|
  puts x
  x == 7
end

puts result

result = [1, 2, 3, 4, 5, 6].find(-> {puts "hello"; 10}) do |x|
  puts x
  x == 1
end

puts result

def foo(&blk)
  result = [1, 2, 3, 4, 5, 6].find(&blk)
  puts result
end

foo do |x|
  puts x
  x.even?
end

arr = [1, 2, 3, 4]
result = arr.find do |x|
  puts x
  arr.pop
  x == 1
end
puts arr
puts result

arr = [1, 2, 3, 4]
result = arr.find do |x|
  puts x
  arr.pop
  x == 10
end
puts arr
puts result
