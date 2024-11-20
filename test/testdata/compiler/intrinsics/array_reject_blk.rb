# frozen_string_literal: true
# typed: true
# compiled: true

result = [1, 2, 3, 4, 5, 6].reject do |x|
  x.even?
end

puts result

puts ([1, 2, 3, 4, 5, 6].reject do |x|
        break "ope"
      end)


def foo(&blk)
  result = [1, 2, 3, 4, 5, 6].reject(&blk)
  puts result
end

foo do |x|
  x.even?
end
