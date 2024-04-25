# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

enumerator = ['zero', 'one', 'two'].reject

# INITIAL-LABEL: define internal i64 @"func_<root>.13<static-init>
# INITIAL: call i64 @sorbet_rb_array_reject(
# INITIAL{LITERAL}: }

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
