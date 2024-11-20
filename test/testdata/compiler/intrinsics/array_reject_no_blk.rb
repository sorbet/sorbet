# frozen_string_literal: true
# typed: true
# compiled: true

enumerator = ['zero', 'one', 'two'].reject


puts enumerator.class

result = enumerator.with_index do |x, i|
  puts "#{i}: #{x}"
end

puts result.class

puts "a"
puts enumerator.each { |x| true }
puts "b"
puts enumerator.each { |x| false }
puts "c"
puts enumerator.each { |x| x.length > 3 }
puts "d"
puts enumerator.each { |x| x.length <= 3 }
