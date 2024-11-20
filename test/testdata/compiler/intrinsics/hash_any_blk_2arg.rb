# frozen_string_literal: true
# typed: true
# compiled: true

h = {a: 1, b: 2, c: :sym}

# "Normal" block
result = h.any? do |k, v|
  v == :sym
end

p result

result = h.any? do |k, v|
  k == 4
end

p result

result = h.any? do |k, v, blank|
  p blank
  k == 4
end

p result
