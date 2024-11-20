# frozen_string_literal: true
# typed: true
# compiled: true

def foo
  yield
end

def bar(&blk)
  foo(&blk)
end



bar do
  puts "bar"
end
