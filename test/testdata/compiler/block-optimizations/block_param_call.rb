# frozen_string_literal: true
# typed: true
# compiled: true

def baz(&blk)
  blk.call("baz")
end



baz do |s|
  p s
end
