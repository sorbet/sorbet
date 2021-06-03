# frozen_string_literal: true
# typed: true
# compiled: true

xs = T.let(['zero', 'one', 'two'], T::Array[String])
enumerator = xs.map!

puts enumerator.class

enumerator.with_index do |x, i|
  puts "#{i}: #{x}"
  [i, x]
end

puts xs
puts xs.class

