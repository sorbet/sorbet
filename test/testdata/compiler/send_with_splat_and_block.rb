# compiled: true
# typed: true
# frozen_string_literal: true

def foo(a, b, c, &blk)
  p a
  p b
  p c
  blk.call(a, b, c)
end

def forward(&blk)
  args = [1,2,3]

  # this produces a call to `<Magic>.<call-with-splat-and-block-pass>`
  foo(*args, &blk)
end

forward do |a,b,c|
  puts "block!: #{a} #{b} #{c}"
end
