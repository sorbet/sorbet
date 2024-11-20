# frozen_string_literal: true
# typed: true
# compiled: true

# Verify that we do reify the block when it's invoked in a rescue block.

def boo(&blk)
  begin
    p "in begin"
    raise 'foo'
  rescue
    p "in rescue"
    yield
  end
end




boo do
  puts "boohey"
end

