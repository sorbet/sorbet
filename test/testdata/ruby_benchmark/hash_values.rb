# frozen_string_literal: true
# typed: strong
# compiled: true
h = {}

10000.times do |i|
  h[i] = nil
end

5000.times do
  h.values
end
