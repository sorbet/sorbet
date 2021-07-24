# typed: true
# frozen_string_literal: true
# compiled: true

x = T.let((1..10).map{|a| [a,nil]}.flatten.to_a, T::Array[T.nilable(Integer)])

res = x.compact!

puts res.nil?
puts x.size
