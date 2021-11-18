# typed: true
# compiled: true
# frozen_string_literal: true

enumerator = ['zero', 'one', 'two'].filter

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

enumerator = ['zero', 'one', 'two'].select

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
