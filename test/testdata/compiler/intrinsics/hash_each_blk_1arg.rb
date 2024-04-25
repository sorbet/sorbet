# frozen_string_literal: true
# typed: true
# compiled: true

# single-arg blocks receive [key, value] arrays
result = {a: 1, b: 2, c: :sym}.each do |a|
  p " #{a}"
end

p result
