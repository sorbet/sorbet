# frozen_string_literal: true
# compiled: true
# typed: true

x = "hi"

i = 0

while i < 10_000_000
  x.to_s

  i += 1
end

puts x
