# frozen_string_literal: true
# typed: true
# compiled: true

enumerator = ['zero', 'one', 'two'].map

puts enumerator.class

result = enumerator.with_index do |x, i|
  puts "#{i}: #{x}"
  [i, x]
end

puts result
puts result.class

