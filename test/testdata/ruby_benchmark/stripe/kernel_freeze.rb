# frozen_string_literal: true
# compiled: true
# typed: true

i = 0
while i < 10_000_000 do
  [i, i, i, i].freeze

  i += 1
end

puts i
