# frozen_string_literal: true
# typed: true
# compiled: true

xs = T.let((0..10).map{|x| [x,x]}.to_a, T::Array[[Integer, Integer]])

i = 0
while i < 1_000_000
  i += 1

  xs.to_h
end

puts i
