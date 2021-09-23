# frozen_string_literal: true
# typed: true
# compiled: true

def f(&blk)
  blk.call(42, &blk)
end

f { |*args, &blk| puts "hi" }
