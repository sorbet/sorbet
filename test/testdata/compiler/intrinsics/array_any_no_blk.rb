# typed: true
# compiled: true
# frozen_string_literal: true

result = ['zero', 'one', 'two'].any?(Integer)
puts result

result = ['zero', 'one', 'two'].any?(String)
puts result

result = [nil, false].any?
puts result

result = [nil, false, 1].any?
puts result

result = ['zero', 1, 'two'].any?(Integer)
puts result

puts [1,2,3].any?(1)

puts [0,2,3].any?(1)
