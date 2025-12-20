# frozen_string_literal: true
# typed: strict
# compiled: true
h = {}

10000.times do |i|
  h[i] = nil
end

50000.times do
  k, v = h.shift
  h[k] = v
end
