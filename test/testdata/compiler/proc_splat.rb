# compiled: true
# typed: true
# frozen_string_literal: true

foo = proc do |*args|
  puts args
end

bar = proc do |x, y, *args|
  puts x, y
  puts args
end

foo.call(1,2,3,'hi')
foo.call
bar.call(1,2,3,'hi')
bar.call(1,2)
