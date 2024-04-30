# frozen_string_literal: true
# typed: true
# compiled: true

# Verify that we do reify the block when it's invoked in a block, even inside begin.

def boo(array, &blk)
  array.each do |x|
    begin
      p x
      yield
    ensure
      p "in ensure"
    end
  end
end




boo([1, 2]) do
  puts "boohey"
end

