# frozen_string_literal: true
# typed: true
# compiled: true

h = {a: 1, b: 2, c: :sym}

result = h.any?([:a, 1])

p result

result = h.any?([:d, 4])

p result

result = h.any?

p result
