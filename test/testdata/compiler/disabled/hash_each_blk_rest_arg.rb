# frozen_string_literal: true
# typed: true
# compiled: true

result = {a: 1, b: 2, c: :sym}.each do |*r|
  p " #{r}"
end

p result
