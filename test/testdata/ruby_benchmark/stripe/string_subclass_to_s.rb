# frozen_string_literal: true
# compiled: true
# typed: true

class StringSubclass < String; end

x = StringSubclass.new

i = 0

while i < 10_000_000
  x.to_s

  i += 1
end

puts x
