# frozen_string_literal: true
# typed: true
# compiled: true

# Verify that we do reify the block when it's invoked in an ensure block.

def boo(&blk)
  begin
    p "in begin"
  ensure
    p "in ensure"
    yield
  end
end




boo do
  puts "boohey"
end

