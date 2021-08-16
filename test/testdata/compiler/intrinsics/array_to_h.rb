# typed: true
# compiled: true
# frozen_string_literal: true

xs = T.let([[:foo, 1], [:bar, 2]], T::Array[[Symbol, Integer]])

hash = xs.to_h

puts hash
puts hash.keys
puts hash.values
puts hash.size
puts hash[:foo]
puts hash[:bar]
puts hash[:baz]
