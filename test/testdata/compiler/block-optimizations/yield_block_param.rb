# frozen_string_literal: true
# typed: true
# compiled: true

def boo(&blk)
  yield
end



boo do
  puts "boohey"
end

