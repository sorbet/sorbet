# frozen_string_literal: true
# typed: true
# compiled: true

h = {a: 1, b: 2, c: :sym}

# single-arg blocks receive [key, value] arrays
result = h.any? do |a|
  a[1] == 2
end

p result

result = h.any? do |a|
  a[0] == 3
end

p result
