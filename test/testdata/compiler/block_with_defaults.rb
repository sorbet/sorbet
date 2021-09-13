# frozen_string_literal: true
# typed: true
# compiled: true
def foo(&blk)
  blk.call(1, 2)
end

foo do |a, b, c = 3|
  puts a, b, c
end
