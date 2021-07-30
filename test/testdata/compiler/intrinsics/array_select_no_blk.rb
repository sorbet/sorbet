# typed: true
# compiled: true
# frozen_string_literal: true

enumerator = ['zero', 'one', 'two'].filter

puts enumerator.class

result = enumerator.with_index do |x, i|
  puts "#{i}: #{x}"
end

puts result.class

enumerator = ['zero', 'one', 'two'].select

puts enumerator.class

result = enumerator.with_index do |x, i|
  puts "#{i}: #{x}"
end

puts result.class
