# frozen_string_literal: true
# typed: true
# compiled: true

hash = {a: 10}

i = 0
misses = 0

while i < 10_000_000 do
  i += 1

  hash.fetch(:x) {misses += 1}
  hash.fetch(:a) {misses += 1}
end

p i
p misses
