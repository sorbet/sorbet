# frozen_string_literal: true
# typed: true
# compiled: true

# Verify that we do reify the block when it's invoked in a block.

def boo(array, &blk)
  array.each do |x|
    p x
    yield
  end
end




boo([1, 2]) do
  puts "boohey"
end

