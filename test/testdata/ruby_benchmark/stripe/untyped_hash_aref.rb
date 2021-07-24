# frozen_string_literal: true
# typed: true
# compiled: true

xs = T.let({key: :val}, T.untyped)

i = 0
while i < 10_000_000
  xs[:key]
  i += 1
end

puts xs[:key]
