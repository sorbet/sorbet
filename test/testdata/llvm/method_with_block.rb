# typed: true

def foo
  yield
end

def boo(&blk)
  yield
#  blk.call("boo1")
end


foo do
  puts "heey"
end

boo do
  puts "boohey"
end

