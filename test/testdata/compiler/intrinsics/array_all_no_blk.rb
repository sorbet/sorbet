# typed: true
# compiled: true
# frozen_string_literal: true

result = ['zero', 'one', 'two'].all?(Integer)
puts result

result = ['zero', 'one', 'two'].all?(String)
puts result

result = [nil, false].all?
puts result

result = [nil, false, 1].all?
puts result

result = ['zero', 1, 'two'].all?(Integer)
puts result

puts [1,1,1].all?(1)

puts [1,1,1].all?(1)
