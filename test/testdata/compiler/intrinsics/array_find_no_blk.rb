# typed: true
# compiled: true
# frozen_string_literal: true

enumerator = ['zero', 'one', 'two'].find(-> {10})

puts enumerator.class

result = enumerator.with_index do |x, i|
  puts "#{i}: #{x}"
end
puts result

puts result.class
