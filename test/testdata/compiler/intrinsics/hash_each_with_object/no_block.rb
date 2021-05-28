# frozen_string_literal: true
# typed: true
# compiled: true

h = {a: 1, b: 1, c: 2, d: 3, e: 5, f: 8}

enum = h.each_with_object([])

p enum.class

result = enum.with_index do |x, i|
  x[1] << i
  x[1] << x[0]
end

p result.class
p result
