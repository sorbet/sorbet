# typed: true
# frozen_string_literal: true
# compiled: true

x = T.let((1..10).map{|a| [a,nil]}.flatten.to_a.freeze, T::Array[T.nilable(Integer)])

i = 0
while i < 1_000_000
  i += 1

  x.compact
end

puts i
