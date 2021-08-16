# frozen_string_literal: true
# typed: true
# compiled: true

enumerator = ['zero', 'one', 'two'].each

puts enumerator.class

result = enumerator.with_index do |x, i|
  puts "#{i}: #{x}"
end

puts result.class
