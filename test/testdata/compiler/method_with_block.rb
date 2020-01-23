# frozen_string_literal: true
# typed: true
# compiled: true

def foo
  yield
end

def boo(&blk)
  yield
end

def bar(&blk)
  foo(&blk)
end

def baz(&blk)
  blk.call("baz")
end

foo do
  puts "heey"
end

boo do
  puts "boohey"
end

bar do
  puts "bar"
end

baz do
end
