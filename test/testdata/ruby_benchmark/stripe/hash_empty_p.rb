# frozen_string_literal: true
# typed: true
# compiled: true

hash = {x: 33, y: 27}

i = 0
while i < 100_000_000 do
  i += (hash.empty? ? 0 : 1)
end

p i
