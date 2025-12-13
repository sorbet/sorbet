# frozen_string_literal: true
# typed: true
# compiled: true

xs = T.let([2], T.untyped)

i = 0
while i < 10_000_000
  xs[0]
  i += 1
end

puts xs[0]
