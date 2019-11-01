# typed: true
# typed: true

def foo
  yield
end

#def boo(&blk)
#  yield "boo"
#  blk.call("boo1")
#end


foo do
  puts "heey"
end

#boo do |arg|
#  puts "boohey"
#end

