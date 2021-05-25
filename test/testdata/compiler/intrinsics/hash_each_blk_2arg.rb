# frozen_string_literal: true
# typed: true
# compiled: true

# "Normal" block
result = {a: 1, b: 2, c: :sym}.each do |k, v|
  p " #{k} => #{v}"
end

p result
